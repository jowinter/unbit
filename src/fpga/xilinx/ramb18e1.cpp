/**
 * @file
 * @brief RAMB36E1 block RAM tile.
 */
#include "fpga/xilinx/bram.hpp"

namespace unbit
{
	namespace xilinx
	{
		//------------------------------------------------------------------------------------------
		// RAMB18E1 tiles are (physically) organized at top or bottom half of a corresponding
		// RAMB36E1 tile.
		//

		//------------------------------------------------------------------------------------------
		ramb18e1::ramb18e1(const ramb36e1& ramb36, bool is_top)
			: bram(ramb36.x(), 2u * ramb36.y() + (is_top ? 1u : 0u), 1024u, 16u, 4u,
				   bram_category::ramb18, ramb36.bitstream_offset(), ramb36.slr()),
			  ramb36(ramb36), is_top(is_top)
		{
		}

		//------------------------------------------------------------------------------------------
		ramb18e1::~ramb18e1()
		{
		}

		//------------------------------------------------------------------------------------------
		const std::string& ramb18e1::primitive() const
		{
			static const std::string primitive_name("RAMB18E1");
			return primitive_name;
		}

		//------------------------------------------------------------------------------------------
		size_t ramb18e1::map_to_bitstream(size_t bit_addr, bool is_parity) const
		{
			// FIXME: Delegate to the "parent" RAMB36E1 (and figure out the twist if any)
			if (is_parity)
			{
				// Map parity bit space	(note that data is swapped at 32-bit word level)
				return ramb36.map_to_bitstream(bit_addr + (is_top ? 2048u : 0u), true);
			}
			else
			{
				// Map data bit space (note that data is swapped at 32-bit word level)
				return ramb36.map_to_bitstream(bit_addr + (is_top ? 16384u : 0u), false);
			}
		}
	}	
}
