/**
 * @file
 * @brief Linear address space to block RAM for Xilinx 7-Series FPGAs.
 */
#ifndef FPGA_XILINX_MAPPER_HPP_
#define FPGA_XILINX_MAPPER_HPP_ 1

#include "common.hpp"

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
			private:
				/**
				 * @brief Word size of the input address space.
				 */
				const size_t input_word_size_;

			public:
				mapper(size_t input_word_size);
				~mapper();

				/**
				 * @brief Install a mapping from a linear source address range to block RAM target.
				 * 
				 * @param[in] src_start specifies the start address (first word in range) in the source address space.
				 * @param[in] src_end specifies the end address (last word in range) in the source address space.
				 * @param[in] src_lsb specifies the lowest bit of the source word that should be mapped.
				 * @param[in] src_msb specifies the highest bit of the source word that should be mapped.
				 * @param[in] ram specifies the target block RAM to map to.
				 * @param[in] target_lsb specifies the least significant bit of the target RA
				 */
				//void map(size_t src_start, size_t src_end, size_t src_lsb, size_t src_msb const bram& ram, size_t target_bit, bool target_is_parity);

			private:
				// Non-copyable
				mapper(const mapper&) = delete;
				mapper& operator=(const mapper&) = delete;
			};

		}
	}
}

#endif // #ifndef FPGA_XILINX_MAPPER_HPP_
