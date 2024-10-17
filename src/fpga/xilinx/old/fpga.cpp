/**
 * @file
 * @brief Common infrastructure for Xilinx Virtex-7 series FPGAs (and alike)
 */
#include "fpga/old/xilinx/fpga.hpp"
#include "fpga/old/xilinx/zynq7.hpp"
#include "fpga/old/xilinx/vup.hpp"

namespace unbit
{
	namespace old
	{
		namespace xilinx
		{
			//------------------------------------------------------------------------------------------
			const fpga& fpga_by_idcode(const uint32_t idcode)
			{
				// 1st chance: Zynq-7
				try
				{
					return v7::zynq7::get_by_idcode(idcode);
				}
				catch (std::invalid_argument&)
				{
					// Squelch mismatch
				}

				// 2nd chance: UltraScale+
				try
				{
					return vup::virtex_up::get_by_idcode(idcode);
				}
				catch (std::invalid_argument&)
				{
					// Squelch mismatch
				}

				// Nothing matched
				throw std::invalid_argument("unknown/unsupported Xilinx device (IDCODE not found)");
			}

			//------------------------------------------------------------------------------------------
			fpga::fpga(const std::string& name, uint32_t idcode, size_t num_brams,
					size_t frame_size, size_t readback_offset,
					size_t front_padding, size_t back_padding,
					size_t back_sync_words)
				:  name_(name), idcode_(idcode), num_brams_(num_brams),
				frame_size_(frame_size), readback_offset_(readback_offset),
				front_padding_(front_padding), back_padding_(back_padding),
				back_sync_words_(back_sync_words)
			{
			}

			//------------------------------------------------------------------------------------------
			fpga::~fpga() noexcept
			{
			}

			//------------------------------------------------------------------------------------------
			const bram& fpga::bram_by_loc(bram_category category, unsigned x, unsigned y) const
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
}
