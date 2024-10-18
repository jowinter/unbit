/**
 * @file
 * @brief Common infrastructure for Xilinx FPGAs
 */
#ifndef UNBIT_XILINX_BITSTREAM_ENGINE_HPP_
#define UNBIT_XILINX_BITSTREAM_ENGINE_HPP_ 1

#include <cstdint>
#include <cstddef>

namespace unbit
{
	namespace xilinx
	{
		//------------------------------------------------------------------------------------------
		/**
		* @brief Bitstream processing engine for Xilinx Series-7 and Virtuex Ultrascale FPGAs
		*/
		class bitstream_engine
		{
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
			 * @param cfg_data points to the start of the bitstream data to be processed.
			 *
			 *  It is the caller responsibility to ensure that @p cfg_data is a valid pointer to array
			 *  of @p cfg_length words. This method does not make any attempts to validate the pointer.
			 *
			 *  The bitstream processor guarantees to only access data words in the memory range that
             *  is specified by @p cfg_data and @p cfg_length via @c uint32_t word loads.
			 *
			 *  No data word accesses are made if an empty range (i.e. @p cfg_length equal to zero)
			 *  is specified.
			 *
			 * @param cfg_len indicates the length (number of 32-bit words) in the configuration
			 *   data array.
			 *
			 * @param is_synced indicates if the first word in the bitstream data array contains
			 *   a valid bitstream packet data (true), or if the processing engine should first
			 *   search for a sync word (false).
			 *
			 * @return The total number of bitstream data words that have been completely processed
			*    by the bitstream processor.
			 */
			std::size_t process(const uint32_t *cfg_data, std::size_t cfg_len, bool is_synced = false);

		protected:
			/**
			 * @brief Configuration write packet
			 * 
			 * @param reg is the configuration register to write.
			 *
			 * @param data points to the start of paramater data of this write.
			 *
			 * @param len indicates the length of parameter data (in words).
			 *
			 * @return True if the packet was processed compeltely, or false if the
			 *   bitstream processing should stop at this packet.
			 */
			virtual bool on_config_write(uint32_t reg, const uint32_t *data, std::size_t len);

			/**
			 * @brief Configuration read packet (might be seen in readback streams).
			 * 
			 * @param reg is the configuration register to write.
			 *
			 * @param data points to the start of paramater data of this write.
			 *
			 * @param len indicates the length of parameter data (in words).
			 *
			 * @return True if the packet was processed compeltely, or false if the
			 *   bitstream processing should stop at this packet.
			 */
			virtual bool on_config_read(uint32_t reg, const uint32_t *data, std::size_t len);

			/**
			 * @brief Reserved configuration packet (op=0).
			 * 
			 * @param reg is the configuration register to write.
			 *
			 * @param data points to the start of paramater data of this write.
			 *
			 * @param len indicates the length of parameter data (in words).
			 *
			 * @return True if the packet was processed compeltely, or false if the
			 *   bitstream processing should stop at this packet.
			 */
			virtual bool on_config_rsvd(uint32_t reg, const uint32_t *data, std::size_t len);

			/**
			 * @brief NOP configuration packet.
			 * 
			 * @param reg is the configuration register to write.
			 *
			 * @param data points to the start of paramater data of this write.
			 *
			 * @param len indicates the length of parameter data (in words).
			 *
			 * @return True if the packet was processed compeltely, or false if the
			 *   bitstream processing should stop at this packet.
			 */
			virtual bool on_config_nop(uint32_t reg, const uint32_t *data, std::size_t len);

		private:
			/**
			 * @brief Scan the data word stream for a valid SYNC word.
			 *
			 * This method scans the data word stream for until sequence of one or more
			 * valid SYNC words are found. The stream position is updated to the first
			 * configuration packet after the SYNC word(s).
			 *
			 * @param pos is the current position in the bitstream data word array.
			 *
			 * @param len indicates the maximum number of data words that are available
			 *   at the current position. The bitstream processor guarantees that this
			 *   callback is only called when at least one data word is available.
			 *
			 * @return The total number of data words that were processed by this
			 *   callback. A zero return value signals to the bitstream processing engine
			 *   that processing of the (current) stream should be stopped without
			 *   advancing to the sync phase.
			 */
			std::size_t synchronize(const uint32_t *pos, std::size_t len);

			/**
			 * @brief Extract a packet from the data word stream.
			 * 
			 * @param pos indicates the current position (packet header word) in the
			 *   data word stream.
			 *
			 * @param len indicates the maximum number of data words that are available
			 *   at the current position. The bitstream processor does not perform any
			 *   checks on the consistency of the header word and the remaining length
			 *   of the bitstream (this callback can indicate an error by returning zero).
			 *			 
			  * @return The total number of data words that were processed by this
			 *   callback. A zero return value signals to the bitstream processing engine
			 *   that processing of the (current) stream should be stopped without
			 *   advancing to following packets.
			 */
			std::size_t parse_packet(const uint32_t *pos, std::size_t len);
		};
	}
}

#endif // UNBIT_OLD_XILINX_BITSTREAM_HPP_
