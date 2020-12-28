/**
 * @file
 * @brief Linear address space to block RAM for Xilinx 7-Series FPGAs.
 */
#ifndef FPGA_XILINX_MAPPER_HPP_
#define FPGA_XILINX_MAPPER_HPP_ 1

#include "common.hpp"

#include <list>

namespace fpga
{
	namespace xilinx
	{
		namespace v7
		{
			//--------------------------------------------------------------------------------------------------------------------
			/**
			 * @brief Address space to block RAM mapper.
			 * 
			 * This address mapper translate memory words from a linear (non-segmented) address space to physical block RAM tiles
			 * on an FPGA. The mapper allows holes in the input address space; the input word size is identical over the entire
			 * input address space.
			 */
			class mapper
			{
			public:
				/**
				 * @brief Mapping information for a rectangular bit block.
				 * 
				 * A bitblock describes a rectangular area of bits that is mapped from the source address
				 * space to a rectangular area of bits in the target address space.
				 */
				struct bitblock
				{
				public:
					/**
					 * @brief Linear start address (last word in range) of the bit block.
					 */
					const size_t start;

					/**
					 * @brief Linear end address (last word in range) of the bit block.
					 */
					const size_t end;

					/**
					 * @brief Least significant source bit to be mapped.
					 */
					const unsigned lsb;

					/**
					 * @brief Most significant source bit to be mapped.
					 */
					const unsigned msb;

					/**
					 * @brief The associated block RAM.
					 */
					const bram& ram;

					/**
					 * @brief Bit-offset in the block RAM
					 */
					size_t ram_offset;

					/**
					 * @brief Stride in the block RAM
					 */
					unsigned ram_stride;

					/**
					 * @brief Indicates whether this mapping targets the data (false) or parity (true) bit area.
					 */
					const bool is_parity;

				public:
					bitblock(size_t start, size_t end, unsigned lsb, unsigned msb,
						const bram& ram, size_t ram_offset, unsigned ram_stride, bool is_parity);

					inline ~bitblock()
					{
					}

					/**
					 * @brief Gets the number of bits in a word of this mapping.
					 */
					size_t width() const;

					/**
					 * @brief Gets the number of words covered by this mapping.
					 */
					size_t depth() const;

					/**
					 * @brief Gets the last bit in RAM that is covered by this bitblock.
					 */
					size_t ram_end() const;
				};

			private:
				/**
				 * @brief Per-bitlane mappings.
				 */
				std::vector<std::list<std::shared_ptr<const bitblock>>> bitlanes_;

			public:
				/**
				 * @brief Constructs a new mapper.
				 */
				mapper(size_t input_word_size);

				/**
				 * @brief Disposes a mapper and its associated resources
				 */
				~mapper();

				/**
				 * @brief Adds a mapping from a linear source address range to block RAM target.
				 * 
				 * @remark Overlapping mappings are resolved in last-in first-out order (the last
				 *   mapping that was added takes precedence).
				 *
				 * @param[in] block specifies the mapping to be installed.
				 */
				void add(const bitblock& block);

				/**
				 * @brief Adds a mapping from a linear source address range to block RAM target.
				 */
				inline void add(size_t start, size_t end, unsigned lsb, unsigned msb,
					const bram& ram, size_t ram_offset, unsigned ram_stride, bool is_parity)
				{
					add(bitblock(start, end, lsb, msb, ram, ram_offset, ram_stride, is_parity));
				}

				/**
				 * @brief Clears all existing mappings.				 
				 */
				void clear();

				/**
				 * @brief Gets the input word size of this mapper.
				 */
				size_t input_word_size() const
				{
					return bitlanes_.size();
				}

				/**
				 * @brief Maps a source bit to the corresponding bitblock.
				 */
				std::shared_ptr<const bitblock> map_bit(size_t bit) const;

				/**
				 * @brief Maps a source word to the corresponding bitblocks.
				 */
				std::vector<std::shared_ptr<const bitblock>> map_word(size_t word) const;

				/**
				 * @brief Prints this mapper
				 */
				std::ostream& print(std::ostream& stm) const;

			private:
				// Non-copyable
				mapper(const mapper&) = delete;
				mapper& operator=(const mapper&) = delete;
			};

			/**
			 * @brief Prints a bitblock mapping.
			 */
			std::ostream& operator<< (std::ostream& stm, const mapper::bitblock& block);

			/**
			 * @brief Prints the bitblock mappings of a mapper
			 */
			static inline std::ostream& operator<< (std::ostream& stm, const mapper& mapper)
			{
				return mapper.print(stm);
			}
		}
	}
}

#endif // #ifndef FPGA_XILINX_MAPPER_HPP_
