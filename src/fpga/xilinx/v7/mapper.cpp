/**
 * @file
 * @brief Linear address space to block RAM mapper.
 */
#include "fpga/xilinx/mapper.hpp"

namespace fpga
{
	namespace xilinx
	{
		namespace v7
		{
			//-------------------------------------------------------------------------------------------------------------------
			mapper::mapper(size_t input_word_size)
				: input_word_size_(input_word_size)
			{
				if (input_word_size_ == 0u)
					throw std::invalid_argument("mapper input word size must be greater than zero.");
			}

			//-------------------------------------------------------------------------------------------------------------------
			mapper::~mapper()
			{
			}

			//-------------------------------------------------------------------------------------------------------------------

		}
	}
}
