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
#include <iomanip>

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

			/** @brief Bitstream SYNC word (in decoded packet header format) */
			static const uint32_t SYNC_WORD = (static_cast<uint32_t>(SYNC_PATTERN[0u]) << 24u) |
				(static_cast<uint32_t>(SYNC_PATTERN[1u]) << 16u) |
				(static_cast<uint32_t>(SYNC_PATTERN[2u]) <<  8u) |
				static_cast<uint32_t>(SYNC_PATTERN[3u]);

			//--------------------------------------------------------------------------------------------------------------------
			void bitstream::parse(const std::string& filename, std::function<bool(const packet&)> callback)
			{
				std::ifstream stm(filename, std::ios_base::in | std::ios_base::binary);
				parse(stm, callback);
			}

			//--------------------------------------------------------------------------------------------------------------------
			void bitstream::parse(std::istream& stm, std::function<bool(const packet&)> callback)
			{
				data_vector bs = load_binary_data(stm);
				parse(bs.cbegin(), bs.cend(), callback);
			}

			//--------------------------------------------------------------------------------------------------------------------
			void bitstream::parse(const_byte_iterator start, const_byte_iterator end, std::function<bool(const packet&)> callback)
			{
				size_t slr = 0u;

				for (const_byte_iterator cur = start; cur != end; slr += 1u)
				{
					cur = parse(cur, end, cur - start, slr, callback);
				}
			}

			//--------------------------------------------------------------------------------------------------------------------
			bitstream::const_byte_iterator bitstream::parse(const_byte_iterator start, const_byte_iterator end,
									size_t base_file_offset, size_t slr,
									std::function<bool(const packet&)> callback)
			{
				// Bitstream format with synchronization word and header commands
				//
				// This parsers method designed to process Virtex-7 (and UltraScale+) style bitstream. Very large Xilinx FPGAs (such as
				// the VCU9P and the larger Virtex-7 FPGAs) use stacked-silicon integration (SSI) technology. For these FPGAs the bitstream
				// is organized into a master super logic region (SLR) and one or more slave SLRs.
				//
				// Slave SLRs can be viewed as "sub" bitstreams. Slave SLRs are encapsulate in a special sequence of type1 and
				// type2 commands. The following snippet. Documentation in UG470 is scarce about the exact mechanism of packing.
				//
				// Analyzing bitstreams of the VCU9P UltraScale+ FPGA suggests that there are multiple configuration frames separated
				// by SYNC words.
				//
				// The bit-stream examined on the VC9UP appears to contain 4 such "slave" bitstreams. Three of them seem to carry
				// an FDRI command with configuration frame data (for SLR0, SLR1, SLR2).
				//

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
				bool     current_write = false;
				uint32_t current_reg   = 0xFFFFFFFFu;

				bool done = false;

				packet pkt;
				pkt.reg = 0u;

				while (!done && (cfg_pos != cfg_end))
				{
					pkt.offset = cfg_pos - start;

					// Track the absolue offset and SLR/stream index of this packet
					pkt.storage_offset = pkt.offset + base_file_offset;
					pkt.stream_index   = slr;

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
						pkt.word_count = pkt.hdr & 0x7FFu;

						// Update the currently selected register (and operation)
						current_reg   = pkt.reg;
						current_write = (pkt.op == 0b10);
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

						// Back-annotate from previous type1 packet
						pkt.op  = current_write ? 0b10 : 0b00;
						pkt.reg = current_reg;
					}
					else if (pkt.hdr == SYNC_WORD)
					{
						// SYNC word (next bitstream follows)
						cfg_pos -= 4u;
						break;
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

					// Invoke the packet callback
					done = !callback(pkt);

					if (pkt.op == 0b10 && pkt.reg == 0b11110 && pkt.word_count > 0u)
					{
						// Check for magic sequence (write to reg 0x1e) with non-zero
						// payload that seems to trigger the next bitstream.
						cfg_pos = pkt.payload_start;
						break;
					}
				}

				// Return the final position in the bitstream
				return cfg_pos;
			}

			//--------------------------------------------------------------------------------------------------------------------
			bitstream::bitstream(std::istream& stm, bitstream::format fmt, uint32_t idcode)
				: data_(load_binary_data(stm))
			{
				if (fmt == bitstream::format::bit)
				{
					// Bitstream format with synchronization word and header commands

					// Step 2: Scan for a type 2 FDRI write packet with the correct number of FDRI data words for an uncompressed
					//   bitstream.
					//
					// Reference: [Xilinx UG470; "Configuration Packets"]
					//
					// Algorithm for the frame-data information:
					// - In the first pass we extract all (sub-)bitstreams (via the parse method)
					//   (This may contain more streams than we have SLRs)
					//
					// - In the second pass, after "parse" has completed, we filter the extract
					//   SLR info objects (only retaining the sub-bitstreams that actually have an FDRI write
					//   with frame data as SLRs)
					//

					// Pass 1: Extract everything
					std::vector<slr_info> substreams;

					parse(data_.cbegin(), data_.cend(), [&] (const packet& pkt)
					{
						// Grow the sub-streams array (if needed)
						if (pkt.stream_index >= substreams.size())
						{
							substreams.emplace_back();
						}

						slr_info& self = substreams.at(pkt.stream_index);

						// Latch the sync offset
						if (self.sync_offset == 0xFFFFFFFFu)
						{
							self.sync_offset = pkt.offset;
						}

						if (pkt.op == 0b10u && pkt.reg == 0b01100u && pkt.word_count > 0u)
						{
							// Write to IDCODE register
							const_byte_iterator pos = pkt.payload_start;
							uint32_t extracted_idcode = static_cast<uint32_t>(*pos++) << 24u;
							extracted_idcode |= static_cast<uint32_t>(*pos++) << 16u;
							extracted_idcode |= static_cast<uint32_t>(*pos++) << 8u;
							extracted_idcode |= static_cast<uint32_t>(*pos++);

							if ((self.idcode != 0xFFFFFFFFu) && (self.idcode != extracted_idcode))
							{
								throw std::invalid_argument("mismatch between actual (extracted from bitstream) and expected idcode values");
							}

							self.idcode = extracted_idcode;
						}
						else if (pkt.op == 0b10u && pkt.reg == 0b00010u && pkt.word_count > 0u)
						{
							// Write to FDRI (frame data input) register
							if (self.frame_data_size > 0u)
							{
								throw std::invalid_argument("unsupported bitstream features: found multiple FDRI write commands (compressed bitstream?)");
							}

							// Record the start and size of the frame data
							self.frame_data_offset = pkt.storage_offset + 4u;
							self.frame_data_size   = pkt.payload_end - pkt.payload_start;
						}
						else if (pkt.hdr == 0x30000001u)
						{
							// Record the CRC command
							//
							///! @bug To be replaced by visitor-style rewriter
							self.crc_check_offset = pkt.offset;
						}

						// Continue parsing
						return true;
					});

					// Pass 2: Retain the substreams with a non-empty FDRI write (uncompressed frame data) as SLR
					//
					// NOTE: We currently assume that SLRs always come in proper order (inside the bitstream).
					slrs_.reserve(substreams.size());

					for (const auto& self : substreams)
					{
						if (self.frame_data_size > 0u)
						{
							slrs_.emplace_back(self);
						}
					}

					if (slrs_.size() == 0u)
					{
						throw std::invalid_argument("unsupported bitstream features: bitstream did not contain any frame data slices");
					}
				}
				else if (fmt == format::raw)
				{
					// Raw bitstream data (no leading config packets)

					///! @bug Currently broken for SLR count >1 (should clone size information from a given reference bitstream and/or FPGA)
					slr_info& slr = slrs_.emplace_back();
					slr.frame_data_offset = 0u;
					slr.frame_data_size = data_.size();
					slr.idcode = idcode;
				}
				else
				{
					// Unsupported/invalid format
					throw std::invalid_argument("unsupported feature: requested bitstream input format is not supported");
				}
			}

			//--------------------------------------------------------------------------------------------------------------------
			bitstream::bitstream(bitstream&& other) noexcept
				: slrs_(std::move(other.slrs_)),
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
				for (auto& self : slrs_)
				{
					if (self.crc_check_offset != 0xFFFFFFFFu)
					{
						auto cmd = data_.begin() + self.crc_check_offset - 4u;

						// Replace the CRC command with two NOPs
						*cmd++ = 0x20u; *cmd++ = 0x00u; *cmd++ = 0x00u; *cmd++ = 0x00u;
						*cmd++ = 0x20u; *cmd++ = 0x00u; *cmd++ = 0x00u; *cmd++ = 0x00u;
					}
				}
			}

			//--------------------------------------------------------------------------------------------------------------------
			bitstream::const_byte_iterator bitstream::config_packets_begin(unsigned slr_index) const
			{
				// Start with the first byte following the sync packet
				return data_.cbegin() + slr(slr_index).sync_offset;
			}

			//--------------------------------------------------------------------------------------------------------------------
			bitstream::const_byte_iterator bitstream::config_packets_end(unsigned slr_index) const
			{
				// Pathologic cases of (partially corrupted) bitstreams could show 1-3 extra bytes near the end; we
				// always round down to a lower 4-byte boundary (this guarantees that users of config_packets_begin/end can
				// alway operate in 4-byte steps without any extra checks).

				const size_t max_config_size = data_.size() - slr(slr_index).sync_offset;
				const size_t trailing_extra_bytes = max_config_size % 4u;
				return data_.cbegin() + (max_config_size - trailing_extra_bytes);
			}

			//--------------------------------------------------------------------------------------------------------------------
			bool bitstream::read_frame_data_bit(size_t bit_offset, unsigned slr_index) const
			{
				// Determine and map the source byte offset
				const size_t src_byte_index = map_frame_data_offset(bit_offset / 8u);
				check_frame_data_range(src_byte_index, 1u, slr_index);

				return static_cast<bool>((data_[src_byte_index + slr(slr_index).frame_data_offset] >> (bit_offset % 8u)) & 1u);
			}


			//--------------------------------------------------------------------------------------------------------------------
			void bitstream::write_frame_data_bit(size_t bit_offset, bool value, unsigned slr_index)
			{
				const size_t dst_byte_index = map_frame_data_offset(bit_offset / 8u);
				check_frame_data_range(dst_byte_index, 1u, slr_index);

				if (value)
				{
					// Set the bit
					data_[dst_byte_index + slr(slr_index).frame_data_offset] |= (1u << (bit_offset % 8u));
				}
				else
				{
					// Clear the bit
					data_[dst_byte_index + slr(slr_index).frame_data_offset] &= ~(1u << (bit_offset % 8u));
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
			void bitstream::check_frame_data_range(size_t offset, size_t length, unsigned slr_index) const
			{
				if (offset >= slr(slr_index).frame_data_size || (slr(slr_index).frame_data_size - offset) < length)
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
