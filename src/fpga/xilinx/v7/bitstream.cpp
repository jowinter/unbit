/**
 * @file
 * @brief Xilinx Series-7 bitstream handling infrastructure
 */
#include "fpga/xilinx/bitstream.hpp"

#include <algorithm>
#include <array>
#include <istream>
#include <fstream>
#include <utility>

#include <iostream>

namespace fpga
{
	namespace xilinx
	{
		/**
		 * @brief Common infrastructure for Xilinx Series-7 FPGAs
		 */
		namespace v7
		{
			/** @brief Bitstream SYNC pattern for Series-7 FPGAs (cf. [Xilinx UG470; "Bitstream Composition"]) */
			static const std::array<uint8_t, 4u> SYNC_PATTERN { 0xAAu, 0x99u, 0x55u, 0x66u };

			//--------------------------------------------------------------------------------------------------------------------
			void bitstream::parse(const std::string& filename, std::function<bool(const packet&)> callback)
			{
				std::ifstream stm(filename, std::ios_base::in | std::ios_base::binary);
				parse(stm, callback);
			}

			//--------------------------------------------------------------------------------------------------------------------
			void bitstream::parse(std::istream& stm, std::function<bool(const packet&)> callback)
			{
				data_vector bs = std::move(load_binary_data(stm));
				parse(bs.cbegin(), bs.cend(), callback);
			}

			//--------------------------------------------------------------------------------------------------------------------
			void bitstream::parse(const_byte_iterator start, const_byte_iterator end, std::function<bool(const packet&)> callback)
			{
				// Bitstream format with synchronization word and header commands

				// Step 1: Synchronize with the start of the configuration stream (by scanning for the 0xAA995566 sync. word.)
				//
				// The bitstream typically contains header data, dummy padding, and markers for bus-width auto-detection before
				// the sync word. We keep this extra data without touching it (to allow proper write-back).
				//
				// Reference: [Xilinx UG470; "Bitstream Composition"]

				auto sync_pos = std::search(start, end, SYNC_PATTERN.cbegin(), SYNC_PATTERN.cend());
				if (sync_pos == end)
					throw std::invalid_argument("sync word (0xAA995566) was not found in the bitstream data.");

				// The sync. offset indicates the first byte after the sync word
				size_t sync_offset = (sync_pos - start) + SYNC_PATTERN.size();

				// Step 2: Scan for a type 2 FDRI write packet with the correct number of FDRI data words for an uncompressed
				//   bitstream.
				//
				// Reference: [Xilinx UG470; "Configuration Packets"]

				// Pathologic cases of (partially corrupted) bitstreams could show 1-3 extra bytes near the end; we
				// always round down to a lower 4-byte boundary (this guarantees that users of config_packets_begin/end can
				// alway operate in 4-byte steps without any extra checks).

				const size_t total_size = end - start;
				const size_t max_config_size = total_size - sync_offset;
				const size_t trailing_extra_bytes = max_config_size % 4u;

				auto cfg_pos = start   + sync_offset;
				auto cfg_end = cfg_pos + (max_config_size - trailing_extra_bytes);

				// Register number (always set by type 1 packets, used by type 2 packets)
				bool done = false;
				packet pkt;
				pkt.reg = 0u;


				while (!done && (cfg_pos != cfg_end))
				{
					pkt.offset = cfg_pos - start;

					// Read the packet header
					pkt.hdr = static_cast<uint32_t>(*cfg_pos++) << 24u;
					pkt.hdr |= static_cast<uint32_t>(*cfg_pos++) << 16u;
					pkt.hdr |= static_cast<uint32_t>(*cfg_pos++) <<  8u;
					pkt.hdr |= static_cast<uint32_t>(*cfg_pos++);

					// Decode the packet type
					pkt.packet_type = (pkt.hdr >> 29u) & 0x7u;
					pkt.op = (pkt.hdr >> 27u) & 0x3u;
					pkt.word_count = 0u;

					if (pkt.packet_type == 0x1u)
					{
						// Type 1 packet:
						//
						//  31 29 28 27 26       18 17  13 12  11 10                  0
						// +-----+-----+-----------+------+------+---------------------+
						// | 001 |  op | 000000000 | reg  |  00  | word_count          |
						// +-----+-----+-----------+------+------+---------------------+
						//
						pkt.reg = (pkt.hdr >> 13u) & 0x1Fu;
						pkt.word_count = pkt.hdr & 0x3FFu;
					}
					else if (pkt.packet_type == 0x2u)
					{
						// Type 2 packet:
						//
						//  31 29 28 27 26                                            0
						// +-----+-----+-----------------------------------------------+
						// | 010 |  op | word_count                                    |
						// +-----+-----+-----------------------------------------------+
						//
						pkt.word_count = pkt.hdr & 0x07FFFFFFu;
					}
					else
					{
						// Unknown packet type
						throw std::invalid_argument("unsupport/unknown configuration packet");
					}

					// Compute data length, skip over the byte count if needed
					size_t byte_count = static_cast<size_t>(pkt.word_count) * 4u;
					if (byte_count > static_cast<size_t>(cfg_end - cfg_pos))
						throw std::invalid_argument("malformed bitstream: packet size exceeds end of bitstream");

					pkt.payload_start = cfg_pos;

					// Advance the position in the config stream
					cfg_pos += byte_count;
					pkt.payload_end = cfg_pos;

					// Invoke the packet callbacek
					done = !callback(pkt);
				}
			}

			//--------------------------------------------------------------------------------------------------------------------
			bitstream::bitstream(std::istream& stm, bitstream::format fmt, uint32_t idcode)
				: sync_offset_(0xFFFFFFFFu),
				  frame_data_offset_(0), frame_data_size_(0),
				  idcode_(idcode),
				  crc_check_offset_(0xFFFFFFFFu),
				  data_(load_binary_data(stm))
			{
				if (fmt == bitstream::format::bit)
				{
					// Bitstream format with synchronization word and header commands

					// Step 2: Scan for a type 2 FDRI write packet with the correct number of FDRI data words for an uncompressed
					//   bitstream.
					//
					// Reference: [Xilinx UG470; "Configuration Packets"]

					parse(data_.cbegin(), data_.cend(), [&] (const packet& pkt)
					{
						// Latch the sync offset
						if (sync_offset_ == 0xFFFFFFFFu)
						{
							sync_offset_ = pkt.offset;
						}

						if (pkt.op == 0b10u && pkt.reg == 0b00100u && pkt.word_count == 1u)
						{
							// Write to CMD register
							const_byte_iterator pos = pkt.payload_start;
							uint32_t cmd = static_cast<uint32_t>(*pos++) << 24u;
							cmd |= static_cast<uint32_t>(*pos++) << 16u;
							cmd |= static_cast<uint32_t>(*pos++) << 8u;
							cmd |= static_cast<uint32_t>(*pos++);

							if (cmd == 0b01101)
							{
								// DESYNC (we definitely reached the end)
								return false;
							}
							else
							{
								// Other command (ignored for noew)
							}
						}
						else if (pkt.op == 0b10u && pkt.reg == 0b01100u && pkt.word_count > 0u)
						{
							// Write to IDCODE register
							const_byte_iterator pos = pkt.payload_start;
							uint32_t extracted_idcode = static_cast<uint32_t>(*pos++) << 24u;
							extracted_idcode |= static_cast<uint32_t>(*pos++) << 16u;
							extracted_idcode |= static_cast<uint32_t>(*pos++) << 8u;
							extracted_idcode |= static_cast<uint32_t>(*pos++);

							if ((idcode_ != 0xFFFFFFFFu) && (idcode_ != extracted_idcode))
							{
								throw std::invalid_argument("mismatch between actual (extracted from bitstream) and expected idcode values");
							}

							idcode_ = extracted_idcode;
						}
						else if (pkt.op == 0b10u && pkt.reg == 0b00010u && pkt.word_count > 0u)
						{
							// Write to FDRI (frame data input) register
							if (frame_data_size_ > 0u)
								throw std::invalid_argument("unsupported bitstream features: found multiple FDRI write commands (compressed bitstream?)");

							// Record the start and size of the frame data
							frame_data_offset_ = pkt.offset + 4u;
							frame_data_size_ = pkt.payload_end - pkt.payload_start;
						}
						else if (pkt.hdr == 0x30000001u)
						{
							// Record the CRC command
							crc_check_offset_ = pkt.offset;
						}

						// Continue parsing
						return true;
					});
				}
				else if (fmt == format::raw)
				{
					// Raw bitstream data (no leading config packets)
					frame_data_offset_ = 0u;
					frame_data_size_ = data_.size();
				}
				else
				{
					// Unsupported/invalid format
					throw std::invalid_argument("unsupported feature: requested bitstream input format is not supported");
				}
			}

			//--------------------------------------------------------------------------------------------------------------------
			bitstream::bitstream(bitstream&& other) noexcept
				: sync_offset_(other.sync_offset_),
				  frame_data_offset_(other.frame_data_offset_), frame_data_size_(other.frame_data_size_),
				  idcode_(other.idcode_),
				  data_(std::move(other.data_))
			{
			}

			//--------------------------------------------------------------------------------------------------------------------
			bitstream::~bitstream() noexcept
			{
			}

			//--------------------------------------------------------------------------------------------------------------------
			void bitstream::update_crc()
			{
				// FIXME: HACK: We currently "update" the CRC check command into to NOOPs
				if (crc_check_offset_ != 0xFFFFFFFFu)
				{
					auto cmd = data_.begin() + crc_check_offset_ - 4u;
					*cmd++ = 0x20u; *cmd++ = 0x00u; *cmd++ = 0x00u; *cmd++ = 0x00u;
					*cmd++ = 0x20u; *cmd++ = 0x00u; *cmd++ = 0x00u; *cmd++ = 0x00u;
				}
			}

			//--------------------------------------------------------------------------------------------------------------------
			bitstream::const_byte_iterator bitstream::config_packets_begin() const
			{
				// Start with the first byte following the sync packet
				return data_.cbegin() + sync_offset_;
			}

			//--------------------------------------------------------------------------------------------------------------------
			bitstream::const_byte_iterator bitstream::config_packets_end() const
			{
				// Pathologic cases of (partially corrupted) bitstreams could show 1-3 extra bytes near the end; we
				// always round down to a lower 4-byte boundary (this guarantees that users of config_packets_begin/end can
				// alway operate in 4-byte steps without any extra checks).

				const size_t max_config_size = data_.size() - sync_offset_;
				const size_t trailing_extra_bytes = max_config_size % 4u;
				return data_.cbegin() + (max_config_size - trailing_extra_bytes);
			}

			//--------------------------------------------------------------------------------------------------------------------
			bool bitstream::read_frame_data_bit(size_t bit_offset) const
			{
				// Determine and map the source byte offset
				const size_t src_byte_index = map_frame_data_offset(bit_offset / 8u);
				check_frame_data_range(src_byte_index, 1u);

				return static_cast<bool>((data_[src_byte_index + frame_data_offset_] >> (bit_offset % 8u)) & 1u);
			}


			//--------------------------------------------------------------------------------------------------------------------
			void bitstream::write_frame_data_bit(size_t bit_offset, bool value)
			{
				const size_t dst_byte_index = map_frame_data_offset(bit_offset / 8u);
				check_frame_data_range(dst_byte_index, 1u);

				if (value)
				{
					// Set the bit
					data_[dst_byte_index + frame_data_offset_] |= (1u << (bit_offset % 8u));
				}
				else
				{
					// Clear the bit
					data_[dst_byte_index + frame_data_offset_] &= ~(1u << (bit_offset % 8u));
				}
			}

			//--------------------------------------------------------------------------------------------------------------------
			bitstream bitstream::load(const std::string& filename, bitstream::format fmt, uint32_t idcode)
			{
				std::ifstream stm(filename, std::ios_base::in | std::ios_base::binary);
				return bitstream(stm, fmt, idcode);
			}

			//--------------------------------------------------------------------------------------------------------------------
			void bitstream::save(const std::string& filename, const bitstream& bs)
			{
				std::ofstream stm(filename, std::ios_base::out | std::ios_base::binary);
				bs.save(stm);
			}

			//--------------------------------------------------------------------------------------------------------------------
			void bitstream::save(std::ostream& f) const
			{
				static_assert(sizeof(std::ostream::char_type) == sizeof(uint8_t), "unsupported host system: sizeof(std::ostream::char_type) != sizeof(uint8_t)");

				f.write(reinterpret_cast<const std::ostream::char_type*>(data_.data()), data_.size());

				if (f.fail())
					throw std::ios_base::failure("i/o error while writing bitstream data to disk.");
			}

			//--------------------------------------------------------------------------------------------------------------------
			bitstream::data_vector bitstream::load_binary_data(std::istream& f)
			{
				// Step 1: Determine the remaining length in the input stream
				size_t size;
				{
					auto start = f.tellg();
					f.seekg(0, std::ios_base::end);

					auto end = f.tellg();

					// Rewind to the initial file position (this allows to user to manually skip extra bytes at the start of f).
					f.seekg(start, std::ios_base::beg);

					if (f.fail())
						throw std::ios_base::failure("i/o error while determining size of bitstream.");

					size = static_cast<size_t>(end - start);
				}

				// Step 2: Read the inomcing data into memory
				bitstream::data_vector raw_data(size);
				{
					static_assert(sizeof(std::istream::char_type) == sizeof(uint8_t), "unsupported host system: sizeof(std::istream::char_type) != sizeof(uint8_t)");

					f.read(reinterpret_cast<std::istream::char_type*>(raw_data.data()), size);

					if (f.fail())
						throw std::ios_base::failure("i/o error while reading bitstream data into memory.");
				}

				return raw_data;
			}

			//--------------------------------------------------------------------------------------------------------------------
			void bitstream::check_frame_data_range(size_t offset, size_t length) const
			{
				if (offset >= frame_data_size_ || (frame_data_size_ - offset) < length)
					throw std::out_of_range("frame data slice is out of bounds");
			}

			//--------------------------------------------------------------------------------------------------------------------
			size_t bitstream::map_frame_data_offset(size_t offset) const
			{
				// Frame data (e.g. bram) is byte-swapped
				const size_t aligned_byte_offset = offset & ~static_cast<size_t>(3u);
				return aligned_byte_offset + (3u - (offset & 3u));
			}
		}
	}
}
