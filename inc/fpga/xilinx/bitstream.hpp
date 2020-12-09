/**
 * @file
 * @brief Common infrastructure for Xilinx Series-7 FPGAs
 */
#ifndef FPGA_XILINX_BITSTREAM_HPP_
#define FPGA_XILINX_BITSTREAM_HPP_ 1

#include "common.hpp"

namespace fpga
{
	namespace xilinx
	{
		namespace v7
		{

			//--------------------------------------------------------------------------------------------------------------------
			/**
			 * @brief Bitstream manipulation for Series-7 FPGAs
			 */
			class bitstream
			{
			public:
				/** Storage vector type. */
				typedef std::vector<uint8_t> data_vector;

				/** Byte data iterator (non-const) */
				typedef data_vector::iterator byte_iterator;

				/** Byte data iterator (const) */
				typedef data_vector::const_iterator const_byte_iterator;

			private:
				/** @brief Byte offset of the first byte following the sync word. */
				size_t sync_offset_;

				/** @brief Byte offset of the first byte of the config frames are. */
				size_t frame_data_offset_;

				/** @brief Size of the config frame data in bytes. */
				size_t frame_data_size_;

				/** @brief IDCODE extracted from the bitstream (or 0xFFFFFFFF if no IDCODE was found) */
				uint32_t idcode_;

				/** @brief In-memory data of the bitstream */
				data_vector data_;

			public:
				/**
				 * @brief Loads an uncompressed (and unencrypted) bitstream from a given file.
				 *
				 * @param[in] filename specifies the name (and path) of the bitstream file to load.
				 *
				 * @return The loaded bitstream object.
				 */
				static bitstream load(const std::string& filename);

			public:
				/**
				 * @brief Constructs a bitstream from a given input stream.
				 *
				 * @param[in] stm is the input stream to read the bitstream data from. The input stream
				 *   should be opened in binary mode.
				 */
				bitstream(std::istream& stm);

				/**
				 * @brief Move constructor for bitstream objects.
				 */
				bitstream(bitstream&& other) noexcept;

				/**
				 * @brief Disposes a bitstream object and its resources.
				 */
				~bitstream() noexcept;

				/**
				 * @brief Gets the byte offset from the start of the bitstream data to the first byte of the FPGA
				 *   configuration frames.
				 */
				inline size_t frame_data_offset() const
				{
					return frame_data_offset_;
				}

				/**
				 * @brief Gets the size of the FPGA configuration frame data in bytes.
				 */
				inline size_t frame_data_size() const
				{
					return frame_data_size_;
				}

				/**
				 * @brief Gets a byte iterator to the begin of the config packets area. (const)
				 */
				const_byte_iterator config_packets_begin() const;

				/**
				 * @brief Gets a byte iterator to the end of the config packets area. (const)
				 */
				const_byte_iterator config_packets_end() const;

				/**
				 * @brief Gets a byte iterator to the begin of the frame data area. (non-const)
				 */
				inline byte_iterator frame_data_begin()
				{
					return data_.begin() + frame_data_offset_;
				}

				/**
				 * @brief Gets a byte iterator to the end of the frame data area. (non-const)
				 */
				inline byte_iterator frame_data_end()
				{
					return frame_data_begin() + frame_data_size_;
				}

				/**
				 * @brief Gets a byte iterator to the begin of the frame data area. (const)
				 */
				inline const_byte_iterator frame_data_begin() const
				{
					return data_.cbegin() + frame_data_offset_;
				}

				/**
				 * @brief Gets a byte iterator to the end of the frame data area. (const)
				 */
				inline const_byte_iterator frame_data_end() const
				{
					return frame_data_begin() + frame_data_size_;
				}

				/**
				 * @brief Reads a bit from the frame data area.
				 * 
				 * @param[in] bit_offset specifies the offset (in bits) relative to the start of the frame data area.
				 *   (the method internally handles 32-bit word swaps as needed)
				 * 
				 * @return The read-back value of the bit.
				 */
				bool read_frame_data_bit(size_t bit_offset) const;

				/**
				 * @brief Writes a bit in the frame data area.
				 * 
				 * @param[bit] bit_offset specifies the offset (in bits) relative to the start of the frame data area.
				 *   (the method internally handles 32-bit word swaps as needed)
				 *
				 * @param[in] value the value to write at the given location.
				 */
				void write_frame_data_bit(size_t bit_offset, bool value);
				
				/**
				 * @brief Gets the device IDCODE that was parsed from the bitstream's configuration packets.
				 */
				inline uint32_t idcode() const
				{
					return idcode_;
				}

			private:
				/**
				 * @brief Helper to load a binary data array from an input stream.
				 *
				 * @return An byte data vector with the loaded binary data.
				 */
				static data_vector load_binary_data(std::istream& stm);

				/**
				 * @brief Performs a range check for a slice of the frame data range.
				 */
				void check_frame_data_range(size_t offset, size_t length) const;

				/**
				 * @brief Remaps a byte offset into the frame data area (adjusting for 32-bit word swaps as needed).
				 *
				 * @param[in] offset is the (byte) offset into the frame data area to be adjusted.
				 * @return The mapped offset (adjusted for 32-bit word swap if needed).
				 */
				size_t map_frame_data_offset(size_t offset) const;

			private:
				// Non-copyable				
				bitstream(const bitstream& other) = delete;
				bitstream& operator=(const bitstream& other) = delete;
			};
		}
	}
}

#endif // FPGA_XILINX_BITSTREAM_HPP_
