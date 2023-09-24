/**
 * @file
 * @brief Infrastructure for Xilinx Virtex UltraScale+ FPGAs
 */
#include "fpga/xilinx/vup.hpp"
#include "fpga/xilinx/bram.hpp"

#include <array>

namespace fpga
{
	namespace xilinx
	{
		namespace vup
		{
			//--------------------------------------------------------------------------------------------------------------------
			//
			// Known Virtex UltraScale+ FPGAs
			//

			/**
			 * @brief Description of a known UltraScale+ variant.
			 */
			struct virtex_up_known_variant
			{
			public:
				/**
				 * @brief Matches this device against the given IDCODE.
				 */
				bool (* const match)(uint32_t idcode);

				/**
				 * @brief Gets the device variant.
				 */
				const virtex_up& (* const get)();
			};

			/**
			 * @brief Array of known UltraScale+ variants.
			 */
			static const std::array<virtex_up_known_variant, 3u> virtex_up_variants
			{
				virtex_up_known_variant { &xcvu9p::match, &xcvu9p::get }, // XCVU9P
			};

			//--------------------------------------------------------------------------------------------------------------------
			virtex_up::virtex_up(const std::string& name, uint32_t idcode, size_t num_brams)
				:  name_(name), idcode_(idcode), num_brams_(num_brams)
			{
			}

			//--------------------------------------------------------------------------------------------------------------------
			virtex_up::~virtex_up() noexcept
			{
			}

			//--------------------------------------------------------------------------------------------------------------------
			const bram& virtex_up::bram_by_loc(bram_category category, unsigned x, unsigned y) const
			{
				for (size_t i = 0u, num_rams = num_brams(category); i < num_rams; ++i)
				{
					const bram& candidate = bram_at(category, i);

					if (candidate.x() == x && candidate.y() == y)
						return candidate;
				}

				throw std::invalid_argument("invalid block ram x/y coordinates.");
			}

			//--------------------------------------------------------------------------------------------------------------------
			const virtex_up& virtex_up::get_by_idcode(uint32_t idcode)
			{
				// Check all known UltraScale+ FPGAs
				for (const auto& variant : virtex_up_variants)
				{
					if (variant.match(idcode))
						return variant.get();
				}

				// Nothing matched
				throw std::invalid_argument("unknown/unsupported UltraScale+ device (IDCODE not found)");
			}
		}
	}
}
