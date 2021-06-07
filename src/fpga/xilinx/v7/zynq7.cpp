/**
 * @file
 * @brief Infrastructure for Xilinx Zynq-7000 FPGAs
 */
#include "fpga/xilinx/zynq7.hpp"
#include "fpga/xilinx/bram.hpp"

#include <array>

namespace fpga
{
	namespace xilinx
	{
		namespace v7
		{
			//--------------------------------------------------------------------------------------------------------------------
			//
			// Known Zynq-7 FPGAs
			//

			/**
			 * @brief Description of a known Zynq-7 variant.
			 */
			struct zynq7_known_variant
			{
			public:
				/**
				 * @brief Matches this device against the given IDCODE.
				 */
				bool (* const match)(uint32_t idcode);

				/**
				 * @brief Gets the device variant.
				 */
				const zynq7& (* const get)();
			};

			/**
			 * @brief Array of known Zynq-7 variants.
			 */
			static const std::array<zynq7_known_variant, 2u> zynq7_variants
			{
				zynq7_known_variant { &xc7z010::match, &xc7z010::get }, // XC7Z010
				zynq7_known_variant { &xc7z020::match, &xc7z020::get }  // XC7Z020
			};

			//--------------------------------------------------------------------------------------------------------------------
			zynq7::zynq7(const std::string& name, uint32_t idcode, size_t num_brams)
				:  name_(name), idcode_(idcode), num_brams_(num_brams)
			{
			}

			//--------------------------------------------------------------------------------------------------------------------
			zynq7::~zynq7() noexcept
			{
			}

			//--------------------------------------------------------------------------------------------------------------------
			const bram& zynq7::bram_by_loc(bram_category category, unsigned x, unsigned y) const
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
			const zynq7& zynq7::get_by_idcode(uint32_t idcode)
			{
				// Check all known Zynq-7 FPGAs
				for (const auto& variant : zynq7_variants)
				{
					if (variant.match(idcode))
						return variant.get();
				}

				// Nothing matched
				throw std::invalid_argument("unknown/unsupported Zynq-7 device (IDCODE not found)");
			}
		}
	}
}
