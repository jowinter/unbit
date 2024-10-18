/**
 * @file
 * @brief Common infrastructure for Xilinx FPGAs
 */
#include "fpga/xilinx/bitstream_engine.hpp"

#include <algorithm>
#include <cassert>

namespace unbit
{
	namespace xilinx
	{
		//------------------------------------------------------------------------------------------
		bitstream_engine::bitstream_engine()
		{
		}

		//------------------------------------------------------------------------------------------
		bitstream_engine::~bitstream_engine()
		{
		}

		//------------------------------------------------------------------------------------------
		std::size_t bitstream_engine::process(const uint32_t *cfg_data, std::size_t cfg_len, bool is_synced)
		{
			if (cfg_len == 0u)
			{
				// Empty data array (cfg_data may be invalid)
				return 0u;
			}


			// Data words are available
			const uint32_t *pos = cfg_data;
			const uint32_t *end = cfg_data + cfg_len;

			// Synchronize first (if needed)
			if (!is_synced)
			{
				std::size_t n_sync = synchronize(pos, end - pos);
				if (n_sync == 0)
				{
					// Pre-sync callback requested the scan to stop.
					return (pos - cfg_data);
				}
				else
				{
					// Update the position (after sync)
					assert (n_sync <= end - pos);
					pos += n_sync;
				}
			}

			// Process packets (until the stream is exhausted, or the callback request an abort)
			while (pos != end)
			{
				std::size_t n_pkt = parse_packet(pos, end - pos);
				if (n_pkt == 0)
				{
					// Packet callback requested the scan to stop.
					return (pos - cfg_data);
				}
				else
				{
					// Update the position (after the packet has been processed)
					assert (n_pkt <= end - pos);
					pos += n_pkt;
				}
			}

			// Scan done (stream exhausted)
			return (pos - cfg_data);
		}

		//------------------------------------------------------------------------------------------
		std::size_t bitstream_engine::synchronize(const uint32_t *pos, std::size_t len)
		{
			const uint32_t *const start = pos;
			const uint32_t *const end   = pos + len;

			// Scan for the first sync word
			uint32_t sync_word = 0u;
			while ((pos != end) && (sync_word != FPGA_SYNC_WORD_LE))
			{
				sync_word = *pos++;
			}

			// Skip over successive sync words
			while ((pos != end) && (*pos == sync_word))
			{
				pos += 1u;
			}

			return pos - start;
		}

		//------------------------------------------------------------------------------------------
		std::size_t bitstream_engine::parse_packet(const uint32_t *pos, std::size_t len)
		{
			const uint32_t *const start = pos;
			const uint32_t *const end = pos + len;

			// Information for the target packet
			uint32_t pkt_reg     = 0u;
			uint32_t pkt_op      = 0u;
			uint32_t word_count  = 0u;

			// Read the packet header and decode the packet type
			const uint32_t hdr = *pos++;

			// Decode the packet type
			uint32_t packet_type = (hdr >> 29u) & 0x7u;

			//
			// Scan first (or only) packet header
			//
			if (hdr == FPGA_SYNC_WORD_LE)
			{
				// Silently tolerate SYNC packets where TYPE1 packets are allowed (for now)
				// TODO: Raise a dedicated event? Or handle DESYNC?
				return (pos - start);
			}
			else if (packet_type == 0x1u)
			{
				// Type 1 packet:
				//
				//  31 29 28 27 26       18 17  13 12  11 10                  0
				// +-----+-----+-----------+------+------+---------------------+
				// | 001 |  op | 000000000 | reg  |  00  | word_count          |
				// +-----+-----+-----------+------+------+---------------------+
				//

				// Type-1 packet
				pkt_op      = (hdr >> 27u) & 0x3u;
				pkt_reg     = (hdr >> 13u) & 0x1Fu;
				word_count  = hdr & 0x3FFu;
			}
			else
			{
				// Unhandled (freestanding) packet (e.g. freestanding TYPE-2)
				return 0;
			}

			// Expect a TYPE2 packet if word count is zero (e.g. for long FDRI writes)
			if (word_count == 0u && pkt_op != 0)
			{
				if (pos == end)
				{
					// Failed to look ahead
					return 0;
				}
				else
				{
					uint32_t hdr_2 = *pos++;
					uint32_t packet_type_2 = (hdr_2 >> 29u) & 0x7u;

					if (packet_type_2 == 0x2u)
					{					
						
						// Type 2 packet:
						//
						//  31 29 28 27 26                                            0
						// +-----+-----+-----------------------------------------------+
						// | 010 |  op | word_count                                    |
						// +-----+-----+-----------------------------------------------+
						//
						word_count = hdr_2 & 0x07FFFFFFu;
					}
					else
					{
						// Unhandled packet!
						return 0;	
					}
				}
			}

			// Capture the data array
			const uint32_t *const data = pos;
			if (word_count > end - data)
			{
				// Malformed bitstream: Word count overflows stream boundaries.#
				return 0u;
			}

			pos += word_count;

			//
			// TYPE-1 packet or TYPE-1/TYPE-2 pait is complete.
			//			

			if (pkt_op == 0b00)
			{
				// 0b00 NOOP - Nothing to do
				if (!on_config_nop(pkt_reg, data, word_count))
				{
					return 0;
				}
			}
			else if (pkt_op == 0b10)
			{
				// 0b10 WRITE
				if (!on_config_write(pkt_reg, data, word_count))
				{
					return 0;
				}
			}
			else if (pkt_op == 0b01)
			{
				// 0b01 READ
				if (!on_config_read(pkt_reg, data, word_count))
				{
					return 0;
				}
			}
			else
			{
				// 0b00 RESERVED (Malformed bitstream; sync packets have been handled above)
				if (!on_config_rsvd(pkt_reg, data, word_count))
				{
					return 0;
				}
			}			

			// Commit the processed information
			return pos - start;
		}

		//------------------------------------------------------------------------------------------
		bool bitstream_engine::on_config_write(uint32_t reg, const uint32_t *data, std::size_t len)
		{
			// Discard unhandled packets by default.
			return true;
		}

		//------------------------------------------------------------------------------------------
		bool bitstream_engine::on_config_read(uint32_t reg, const uint32_t *data, std::size_t len)
		{
			// Reject unhandled read packets by default
			return false;
		}

		//------------------------------------------------------------------------------------------
		bool bitstream_engine::on_config_nop(uint32_t reg, const uint32_t *data, std::size_t len)
		{
			// Discard NOP packets by default.
			return true;
		}

		//------------------------------------------------------------------------------------------
		bool bitstream_engine::on_config_rsvd(uint32_t reg, const uint32_t *data, std::size_t len)
		{
			// Reject reserved packets by default
			return false;
		}
	}
}
