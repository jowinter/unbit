/**
 * @file
 * @brief Bitstream parser engine for Xilinx Series-7 and UltraScale FPGAs.
 */
#ifndef UNBIT_XILINX_BITSTREAM_ENGINE_HPP_
#define UNBIT_XILINX_BITSTREAM_ENGINE_HPP_ 1

#include "unbit/fpga/xilinx/config_reg.hpp"

#include <cstdint>
#include <cstddef>

#include <span>
#include <tuple>
#include <utility>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			//------------------------------------------------------------------------------------------
			/**
			 * @brief Bitstream parser engine for Xilinx Series-7 and Ultrascale FPGAs.
			 *
			 * The @c bitstream_engine class provides basic functionality to parse an FPGA bitstream
			 * into a sequence of configuration packets. The bitstream parser engine translates the raw
			 * configuration packets found in the bitstream into a sequence of configuration events.
			 *
			 * Depending on the number of associated payload data words a configuration event may be
			 * encoded either as a single TYPE1 packet (for "short" payloads), or as a sequence of a
			 * TYPE1 and a TYPE2 packet (for "long" packets such as frame data input register writes),
			 * The bitstream parser engine handles both cases and always produces a single, normalized
			 * configuration event that includes the target register and the payload data.
			 */
			class bitstream_engine
			{
			public:
				/**
				 * @brief Span of configuration words (with dynamic extent).
				 */
				typedef std::span<const uint32_t> word_span_type;

				/**
				 * @brief Iterator type for configuration word spans.
				 */
				typedef word_span_type::iterator word_span_iterator;

				/**
				 * @brief Final bitstream position and completion/success status of a parser operation.
				 */
				typedef std::tuple<word_span_iterator, bool> parser_status_type;

			public:
				/**
				 * @brief FPGA sync word (little-endian).
				 */
				static const uint32_t FPGA_SYNC_WORD_LE = 0xAA995566u;

			protected:
				/**
				 * @brief Construct a new bitstream engine object
				 */
				bitstream_engine();

				/**
				 * @brief Destroy the bitstream engine object
				 */
				~bitstream_engine();

			public:
				/**
				 * @brief Process bitstream packets loaded in memory.
				 *
				 * @param first references the first bitstream data word.
				 * 
				 * @param last references the word following the last bitstream data word.
				 *
				 * @return True if the stream was processed completely, false otherwise.
				 */
				template<typename It, typename End>
				inline std::tuple<std::size_t, bool> process(It first, End last)
				{
					word_span_type data_span(first, last);
					auto [final_pos, success] = process_packets(data_span, false);
					return std::make_tuple(final_pos - data_span.begin(), success);
				}

				/**
				 * @brief Process bitstream packets loaded in memory.
				 *
				 * @param cfg_data specifies the span of bitstream data words to be parsed.
				 *
				 * @return True if the stream was processed completely, false otherwise.
				 */
				inline std::tuple<std::size_t, bool> process(word_span_type cfg_data)
				{
					auto [final_pos, success] = process_packets(cfg_data, false);
					return std::make_tuple(final_pos - cfg_data.begin(), success);
				}

			protected:
				/**
				 * @brief Process bitstream packets loaded in memory.
				 *
				 * @param cfg_data specifies the bitstream data to be parsed.
				 *
				 * @param is_synchronized indicates if the first word in the bitstream data array contains
				 *   a valid bitstream packet data (true), or if the processing engine should first
				 *   search for a sync word (false).
				 *
				 * @return The total number of words that were processed by this method.
				 */
				virtual parser_status_type process_packets(word_span_type cfg_data, bool is_synchronized);

				/**
				 * @brief Scan the data word stream for a valid SYNC word.
				 *
				 * This method scans the data word stream for until sequence of one or more
				 * valid SYNC words are found. The returned span
				 *
				 * @param cfg_data is the span of the bitstream data word array to be scanned.
				 *
				 * @return A reference to the first data word (or the end of span) following the
				 *   synchronization sequence.
				 */
				virtual word_span_iterator synchronize(word_span_type cfg_data) const;

				/**
				 * @brief Parses a configuration packet.
				 *
				 * @param pkt_data specifies the available bitstream data to be parsed as packet.
				 *
				 * @return A reference to the first data word (or the end of span) following the
				 *   parsed packet.
				 */
				virtual parser_status_type parse_packet(word_span_type pkt_data);

				/**
				 * @brief Configuration write packet
				 *
				 * @param reg is the configuration register to write.
				 *
				 * @param data points to the start of paramater data of this write.
				 *
				 * @return True if the packet was processed compeltely, or false if the
				 *   bitstream processing should stop at this packet.
				 */
				virtual bool on_config_write(config_reg reg, word_span_type data);

				/**
				 * @brief Configuration read packet (might be seen in readback streams).
				 *
				 * @param reg is the configuration register to write.
				 *
				 * @param data points to the start of paramater data of this write.
				 *
				 * @return True if the packet was processed compeltely, or false if the
				 *   bitstream processing should stop at this packet.
				 */
				virtual bool on_config_read(config_reg reg, word_span_type data);

				/**
				 * @brief Reserved configuration packet (op=0).
				 *
				 * @param reg is the configuration register to write.
				 *
				 * @param data points to the start of paramater data of this write.
				 *
				 * @return True if the packet was processed compeltely, or false if the
				 *   bitstream processing should stop at this packet.
				 */
				virtual bool on_config_rsvd(config_reg reg, word_span_type data);

				/**
				 * @brief NOP configuration packet.
				 *
				 * @param reg is the configuration register to write.
				 *
				 * @param data points to the start of paramater data of this write.
				 *
				 * @return True if the packet was processed compeltely, or false if the
				 *   bitstream processing should stop at this packet.
				 */
				virtual bool on_config_nop(config_reg reg, word_span_type data);
			};
		}
	}
}

#endif // UNBIT_XILINX_BITSTREAM_ENGINE_HPP_
