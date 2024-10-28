/**
 * @file
 * @brief Common infrastructure for Xilinx FPGAs
 */
#include "unbit/fpga/xilinx/bitstream_engine.hpp"
#include "unbit/fpga/xilinx/bitstream_error.hpp"

#include <algorithm>
#include <cassert>

#include <algorithm>

namespace unbit
{
	namespace fpga
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
			bitstream_engine::parser_status_type bitstream_engine::process_packets(word_span_type cfg_data, bool is_synchronized)
			{
				word_span_iterator pos = cfg_data.begin();
				bool pkt_valid = true;

				// Synchronize the stream (if needed)
				if (!is_synchronized)
				{
					pos = synchronize(cfg_data);
				}

				// Process packets (until the stream is exhausted, or the callback request an abort)
				while (pkt_valid && pos != cfg_data.end())
				{
					std::tie(pos, pkt_valid) = parse_packet(word_span_type(pos, cfg_data.end()));
				}

				return parser_status_type(pos, pkt_valid);
			}

			//------------------------------------------------------------------------------------------
			bitstream_engine::word_span_iterator bitstream_engine::synchronize(word_span_type data) const
			{
				// Scan for the first sync word
				word_span_iterator pos = std::find(data.begin(), data.end(), FPGA_SYNC_WORD_LE);

				// Skip over successive sync words
				while ((pos != data.end()) && (*pos == FPGA_SYNC_WORD_LE))
				{
					++pos;
				}

				// Done
				return pos;
			}

			//------------------------------------------------------------------------------------------
			bitstream_engine::parser_status_type bitstream_engine::parse_packet(word_span_type pkt_data)
			{
				word_span_iterator pos = pkt_data.begin();
				if (pos == pkt_data.end())
				{
					// End of stream (no packet header available)
					return parser_status_type(pos, false);
				}

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
					return parser_status_type(pos, true);
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
					throw bitstream_error("unhandled packet type at current bitstream location");
				}

				// Expect a TYPE2 packet if word count is zero (e.g. for long FDRI writes)
				if (word_count == 0u && pkt_op != 0)
				{
					if (pos == pkt_data.end())
					{
						// Failed to look ahead
						throw bitstream_error("unexpected end of bitstream (expected a type2 packet with payload data)");
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
							throw bitstream_error("unhandled packet type at current bitstream location (expected a type2 packet with payload data)");
						}
					}
				}

				// Capture the data array
				if ((pkt_data.end() - pos) < word_count)
				{
					// Malformed bitstream: Word count overflows stream boundaries.
					throw bitstream_error("payload data size exceeds bitstream boundaries");
				}
				
				word_span_type data_span(pos, word_count);

				// Skip over the data area
				pos += word_count;

				//
				// TYPE-1 packet or TYPE-1/TYPE-2 pair is complete.
				//
				bool success = false;

				if (pkt_op == 0b00)
				{
					// 0b00 NOOP - Nothing to do
					success = on_config_nop(static_cast<config_reg>(pkt_reg), data_span);
				}
				else if (pkt_op == 0b10)
				{
					// 0b10 WRITE
					success = on_config_write(static_cast<config_reg>(pkt_reg), data_span);
				}
				else if (pkt_op == 0b01)
				{
					// 0b01 READ
					success = on_config_read(static_cast<config_reg>(pkt_reg), data_span);
				}
				else
				{
					// 0b00 RESERVED (Malformed bitstream; sync packets have been handled above)
					success = on_config_rsvd(static_cast<config_reg>(pkt_reg), data_span);
				}

				// Commit the processed information
				return parser_status_type(pos, success);
			}

			//------------------------------------------------------------------------------------------
			bool bitstream_engine::on_config_write(config_reg reg, word_span_type data)
			{
				// Discard unhandled packets by default.
				return true;
			}

			//------------------------------------------------------------------------------------------
			bool bitstream_engine::on_config_read(config_reg reg, word_span_type data)
			{
				// Discard unhandled read packets by default
				return true;
			}

			//------------------------------------------------------------------------------------------
			bool bitstream_engine::on_config_nop(config_reg reg, word_span_type data)
			{
				// Discard NOP packets by default.
				return true;
			}

			//------------------------------------------------------------------------------------------
			bool bitstream_engine::on_config_rsvd(config_reg reg, word_span_type data)
			{
				// Reject reserved packets by default
				return false;
			}
		}
	}
}
