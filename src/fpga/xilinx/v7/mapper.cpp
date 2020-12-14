/**
 * @file
 * @brief Linear address space to block RAM mapper.
 */
#include "fpga/xilinx/mapper.hpp"
#include "fpga/xilinx/bram.hpp"

#include <cassert>
#include <stdexcept>
#include <limits>

#include <iostream>
#include <iomanip>

namespace fpga
{
	namespace xilinx
	{
		namespace v7
		{
			//-------------------------------------------------------------------------------------------------------------------
			mapper::bitblock::bitblock(size_t start, size_t end, unsigned lsb, unsigned msb,
				const bram& ram, size_t ram_offset, unsigned ram_stride, bool is_parity)
				: start(start), end(end), lsb(lsb), msb(msb),
				  ram(ram), ram_offset(ram_offset), ram_stride(ram_stride), is_parity(is_parity)
			{
				// TODO: Check for integer overflows!? (add sanity checks)
				if (start > end)
					throw std::invalid_argument("start address of bitblock must be less or equal than end address.");

				if (lsb > msb)
					throw std::invalid_argument("source lsb of bitblock must be less or equal than source msb.");

				if (ram_stride < 1u)
					throw std::out_of_range("target ram stride of bitblock must be greater than zero.");
			
				const size_t width = msb - lsb + 1u;
				if (width > ram_stride)
					throw std::out_of_range("target ram stride of bitblock must be greater or equal than word width.");

				const size_t ram_size = (is_parity ? ram.parity_bits() : ram.data_bits()) * ram.num_words();
				if (ram_offset >= ram_size)
					throw std::out_of_range("target ram offset of bitblock must be less or equal than ram size.");

				if (ram_end() >= ram_size)
					throw std::out_of_range("size and/or stride of bitblock exceed target ram size.");
			}

			//-------------------------------------------------------------------------------------------------------------------
			size_t mapper::bitblock::width() const
			{
				return msb - lsb + 1u;
			}

			//-------------------------------------------------------------------------------------------------------------------
			size_t mapper::bitblock::depth() const
			{
				return end - start + 1u;
			}

			//-------------------------------------------------------------------------------------------------------------------
			size_t mapper::bitblock::ram_end() const
			{
				return ram_offset + (depth() - 1u) * ram_stride + (width() - 1u);
			}

			//-------------------------------------------------------------------------------------------------------------------
			std::ostream& operator<< (std::ostream& stm, const mapper::bitblock& block)
			{		
				return (stm	<< "0x" << std::hex << block.start 
					<< "..0x" << block.end
					<< " [" << std::dec << block.msb << ":" << block.lsb
					<< "] <=> " << block.ram
					<< (block.is_parity ? " PAR 0x" : " DAT 0x")
					<< std::hex << block.ram_offset << "..0x" << block.ram_end()
					<< "(+" << std::dec << block.ram_stride << ")");
			}

			//-------------------------------------------------------------------------------------------------------------------
			mapper::mapper(size_t input_word_size)
				: bitlanes_(input_word_size)
			{
				if (input_word_size == 0u)
					throw std::invalid_argument("mapper input word size must be greater than zero.");
			}

			//-------------------------------------------------------------------------------------------------------------------
			mapper::~mapper()
			{
			}

			//-------------------------------------------------------------------------------------------------------------------
			void mapper::add(const mapper::bitblock& block)
			{
				const size_t num_bitlanes = bitlanes_.size();

				// Sanity: Check word from block must fit (we only need to check the MSB as the block mandates LSB < MSB)
				if (block.msb >= num_bitlanes)
					throw std::out_of_range("bitblock word slice exceeds bounds of mapper word size");

				// Put the new mapping in front
				auto mapped = std::make_shared<mapper::bitblock>(block);
				
				// Now register the new mapping with all affected bitlanes
				for (size_t i = 0u; i < num_bitlanes; ++i)
				{
					if (mapped->lsb <= i && i <= mapped->msb)
					{
						// Mapping intersects the bit-lane; add it in front
						bitlanes_[i].emplace_front(mapped);
					}
				}
			}

			//-------------------------------------------------------------------------------------------------------------------
			void mapper::clear()
			{
				// Clear the per-lane mappings
				for (auto& lane : bitlanes_)
				{
					lane.clear();
				}
			}

			//-------------------------------------------------------------------------------------------------------------------
			std::shared_ptr<const mapper::bitblock> mapper::map_bit(size_t bit) const
			{
				const size_t num_bitlanes = bitlanes_.size();
				const size_t word_index = (bit / num_bitlanes);
				const size_t bit_index = (bit % num_bitlanes);

				for (auto block : bitlanes_[bit_index])
				{
					if (block->start <= word_index && word_index <= block->end)
					{
						// We found a matching bitlane
						return block;
					}
				}

				// No mapping found
				return {};
			}

			//-------------------------------------------------------------------------------------------------------------------
			std::vector<std::shared_ptr<const mapper::bitblock>> mapper::map_word(size_t word) const
			{
				const size_t num_bitlanes = bitlanes_.size();

				std::vector<std::shared_ptr<const mapper::bitblock>> mapped(num_bitlanes);

				for (size_t i = 0u; i < num_bitlanes; ++i)
				{
					// Scan over all mappings in the lane, and yield the first matching one.
					for (auto block : bitlanes_[i])
					{
						if (block->start <= word && word <= block->end)
						{
							// We found a matching bitlane
							mapped[i] = block;
							break;
						}
					}
				}

				return mapped;
			}
		}
	}
}
