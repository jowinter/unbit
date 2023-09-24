/**
 * @file
 * @brief Common infrastructure for Xilinx Virtex-7 series FPGAs (and alike)
 */
#include "fpga/xilinx/xilinx_fpga.hpp"

#include <array>

namespace fpga
{
	namespace xilinx
	{
		//--------------------------------------------------------------------------------------------------------------------
		xilinx_fpga::xilinx_fpga(const std::string& name, uint32_t idcode, size_t num_brams)
			:  name_(name), idcode_(idcode), num_brams_(num_brams)
		{
		}
	  
		//--------------------------------------------------------------------------------------------------------------------
		xilinx_fpga::~xilinx_fpga() noexcept
		{
		}
		
		//--------------------------------------------------------------------------------------------------------------------
		const bram& xilinx_fpga::bram_by_loc(bram_category category, unsigned x, unsigned y) const
		{
			for (size_t i = 0u, num_rams = num_brams(category); i < num_rams; ++i)
			{
				const bram& candidate = bram_at(category, i);
				
				if (candidate.x() == x && candidate.y() == y)
					return candidate;
			}

			throw std::invalid_argument("invalid block ram x/y coordinates.");
		}
	}
}
