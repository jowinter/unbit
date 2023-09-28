/**
 * @file
 * @brief Infrastructure for Xilinx Virtex UltraScale+ FPGAs
 */
#include "fpga/xilinx/vup.hpp"
#include "fpga/xilinx/bram.hpp"

#include <array>

namespace unbit
{
	namespace xilinx
	{
		namespace vup
		{
			//--------------------------------------------------------------------------------------
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
			static const std::array<virtex_up_known_variant, 1u> virtex_up_variants
			{
				virtex_up_known_variant { &xcvu9p::match, &xcvu9p::get }, // XCVU9P
			};

			//--------------------------------------------------------------------------------------
			virtex_up::virtex_up(const std::string& name, uint32_t idcode, size_t num_brams)
				:  fpga(name, idcode, num_brams,
						93u * 4u,         // 93 words per frame
						(20u + 93u) * 4u) // 20 pipeline words + 1 frame readfback offset
			{
			}

			//--------------------------------------------------------------------------------------
			virtex_up::~virtex_up() noexcept
			{
			}

			//--------------------------------------------------------------------------------------
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
