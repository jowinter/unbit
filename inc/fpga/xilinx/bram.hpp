/**
 * @file
 * @brief Common infrastructure for Xilinx Series-7 FPGAs
 */
#ifndef FPGA_XILINX_BRAM_HPP_
#define FPGA_XILINX_BRAM_HPP_ 1

#include "common.hpp"

namespace fpga
{
	namespace xilinx
	{
		namespace v7
		{
			//--------------------------------------------------------------------------------------------------------------------
			/**
			 * @brief Known categories of block RAMs.
			 */
			enum class bram_category
			{
				ramb18,
				ramb36
			};

			//--------------------------------------------------------------------------------------------------------------------
			/**
			 * @brief Description of a block RAM in a 7-Series FPGA (e.g. Zynq)
			 */
			class bram
			{
			protected:
				/**
				 * @brief Super-logic slice (SLR) of this RAM.
				 */
				const unsigned slr_;

				/**
				 * @brief X location of the RAM tile.
				 */
				const unsigned x_;

				/**
				 * @brief Y location of the RAM tile.
				 */
				const unsigned y_;

				/**
				 * @brief Total number of RAM words.
				 */
				const size_t num_words_;

				/**
				 * @brief Number of data bits per RAM word.
				 */
				const size_t data_bits_;

				/**
				 * @brief Number of parity bits per RAM word.
				 */
				const size_t parity_bits_;

				/**
				 * @brief Category of this RAM.
				 */
				const bram_category category_;

				/**
				 * @brief Bit-offset of the first bit of the BRAM in the configuration frame data
				 *   found in the bitstream.
				 *
				 * @note The bit-offset is measured relative to the start of the start of the
				 *   configuration frame data are in the bitstream.
				 */
				size_t bitstream_offset_;

			protected:
				/**
				 * @brief Constructs a block RAM description.
				 *
				 * @param[in] x specifies the X coordinate of the RAM tile.
				 * @param[in] y specifies the Y coordinate of the RAM tile.
				 * @param[in] data_bits specifies the number of data bits per RAM word.
				 * @param[in] parity_bits specifies the number of parity bits per RAM word.
				 * @param[in] category specifies the category of this RAM.
				 * @param[in] bitstream_offset specifies the offset (in bits) between the start of the
				 *   configuration frame data in the bitstream and the first bit related to the block RAM.
				 * @param[in] slr specifies the zero-based index of the SLR bitstream of this RAM.
				 */
				bram(unsigned x, unsigned y, size_t num_words, size_t data_bits, size_t parity_bits,
				     bram_category categroy, size_t bitstream_offset, unsigned slr);

				/**
				 * @brief Diposes a block RAM descriptor
				 */
				virtual ~bram() = 0;

			public:
				/**
				 * @brief Gets the name of the block RAM primitive.
				 */
				virtual const std::string& primitive() const = 0;

				/**
				 * @brief Maps a RAM data (or parity) bit location to the bitstream.
				 *
				 * @param[in] bit_addr is the address of the (data or parity) bit to be mapped.
				 *
				 * @param[in] is_parity indicates whether a data bit (false) or a parity bit (true) is to be mapped.
				 *
				 * @return The bitstream offset of the given RAM bit. The returned offset is a bit-address relative to the
				 *  start of the configuration frame of the bitstream. (An address relative to the RAM can be computed by
				 *  subtracting the bitstream offset of this RAM itself).
				 *
				 * @note The returned value is given relative to the SLR's data frame.
				 */
				virtual size_t map_to_bitstream(size_t bit_addr, bool is_parity) const = 0;

				/**
				 * @brief Extracts data or parity bits of this block RAM from a bitstream.
				 *
				 * @param[in]  bits specifies the source bitstream.
				 * @param[in]  extract_parity indicates whether data (false) or parity (true) data shall be extracted.
				 *
				 * @return A fresh byte vector containing the extracted data bits.
				 */
				std::vector<uint8_t> extract(const bitstream& bits, bool extract_parity) const;

				/**
				 * @brief Injects data or parity bits for this block RAM into a bitstream.
				 *
				 * @param[in,out] bits specifies the target bitstream.
				 * @param[in] inject_parity indicates whether data (false) or parity (true) data shall be injected.
				 * @param[in] data specifies the byte vector to be injected.
				 */
				void inject(bitstream& bits, bool inject_parity, const std::vector<uint8_t>& data) const;

				/**
				 * @brief Gets the SLR index of this RAM tile.
				 */
				inline unsigned slr() const
				{
					return slr_;
				}

				/**
				 * @brief Gets the X location of this RAM tile.
				 */
				inline unsigned x() const
				{
					return x_;
				}

				/**
				 * @brief Gets the Y location of this RAM tile.
				 */
				inline unsigned y() const
				{
					return y_;
				}

				/**
				 * @brief Gets the number of words of this RAM tile.
				 */
				inline size_t num_words() const
				{
					return num_words_;
				}

				/**
				 * @brief Gets the number of data bits per RAM word.
				 */
				inline size_t data_bits() const
				{
					return data_bits_;
				}

				/**
				 * @brief Gets the number of parity bits per RAM word.
				 */
				inline size_t parity_bits() const
				{
					return parity_bits_;
				}

				/**
				 * @brief Gets the category of this RAM.
				 */
				inline bram_category category() const
				{
					return category_;
				}

				/**
				 * @brief Gets the offset between the start of the bitstream's configuration area
				 *   and the first bit related to this block RAM.
				 *
				 * @note The returned value is given relative to the SLR's data frame.
				 */
				inline size_t bitstream_offset() const
				{
					return bitstream_offset_;
				}

			protected:
				/**
				 * @brief Maps a bit address to a bitstream offset (using a provided mapping table).
				 *
				 * @param[in] bit_addr is the bit address to be mapped.
				 * @param[in] map_table is the mapping table (the bit address is used as index).
				 * @param[in] table_size is the number of entries in the mapping table.
				 * @return The mapped bitstream offset.
				 */
				size_t map_via_table(size_t bit_addr, const uint32_t* map_table, size_t table_size) const;

			private:
				// Non-copyable
				bram(const bram&) = delete;
				bram& operator=(const bram&) = delete;
			};

			/**
			 * @brief Prints the type and location of a RAM.
			 *
			 * @param[in,out] stm is the output stream to print to.
			 * @param[in] ram is the RAM to be printed.
			 * @return A reference to the output stream.
			 */
			std::ostream& operator<< (std::ostream& stm, const bram& ram);

			//--------------------------------------------------------------------------------------------------------------------
			/**
			 * @brief Description of a RAMB36E1 block RAM tile.
			 */
			class ramb36e1 final : public bram
			{
			public:
				/**
				 * @brief Construct a RAMB36E1 block RAM tile.
				 */
				ramb36e1(unsigned x, unsigned y, size_t bitstream_offset, unsigned slr = 0u);

				/**
				 * @brief Disposes a RAMB36E1 block RAM tile.
				 */
				~ramb36e1();

			public:
				const std::string& primitive() const override;

				size_t map_to_bitstream(size_t bit_addr, bool is_parity) const override;
			};

			//--------------------------------------------------------------------------------------------------------------------
			/**
			 * @brief Description of a RAMB18E1 block RAM tile.
			 */
			class ramb18e1 final : public bram
			{
			private:
				/**
				 * @brief Enclosing RAMB36E1 primitive.
				 */
				const ramb36e1& ramb36;

				/**
				 * @brief Indicates if this is the top (true) or bottom (false) half.
				 */
				bool is_top;

			public:
				/**
				 * @brief Construct a RAMB18E1 block RAM tile.
				 */
				ramb18e1(const ramb36e1& ramb36, bool is_top);

				/**
				 * @brief Disposes a RAMB36E1 block RAM tile.
				 */
				~ramb18e1();

			public:
				const std::string& primitive() const override;

				size_t map_to_bitstream(size_t bit_addr, bool is_parity) const override;
			};
		}
	}
}

#endif // #ifndef FPGA_XILINX_BRAM_HPP_
