/**
 * @file
 * @brief Common infrastructure for Xilinx FPGAs
 */
#ifndef UNBIT_XILINX_BITSTREAM_ENGINE_HPP_
#define UNBIT_XILINX_BITSTREAM_ENGINE_HPP_ 1

#include <cstdint>
#include <cstddef>

#include <span>
#include <tuple>
#include <utility>

#include <iosfwd>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			//------------------------------------------------------------------------------------------
			/**
			 * @brief Bitstream parsing engine for Xilinx Series-7 and Virtuex Ultrascale FPGAs.
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
				 * @param cfg_data specifies the bitstream data to be parsed.
				 *
				 * @param is_synchronized indicates if the first word in the bitstream data array contains
				 *   a valid bitstream packet data (true), or if the processing engine should first
				 *   search for a sync word (false).
				 *
				 * @return True if the stream was processed completely, false otherwise.
				 */
				template<typename It, typename End>
				inline std::tuple<std::size_t, bool> process(It first, End last)
				{
					word_span_type data_span(first, last);
					auto [final_pos, success] = process(data_span, false);
					return std::make_tuple(final_pos - data_span.begin(), success);
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
				virtual parser_status_type process(word_span_type cfg_data, bool is_synchronized);

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
				virtual bool on_config_write(uint32_t reg, word_span_type data);

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
				virtual bool on_config_read(uint32_t reg, word_span_type data);

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
				virtual bool on_config_rsvd(uint32_t reg, word_span_type data);

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
				virtual bool on_config_nop(uint32_t reg, word_span_type data);
			};
		}
	}
}

#endif // UNBIT_XILINX_BITSTREAM_ENGINE_HPP_
