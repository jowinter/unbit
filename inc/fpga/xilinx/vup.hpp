/**
 * @file
 * @brief Virtex UltraScale+ FPGAs
 */
#ifndef UNBIT_XILINX_VUP_HPP_
#define UNBIT_XILINX_VUP_HPP_ 1

#include "fpga.hpp"

namespace unbit
{
	namespace xilinx
	{
		namespace vup
		{
			//--------------------------------------------------------------------------------------
			/**
			 * @brief Description of a Virtex UltraScale+ FPGA device
			 */
			class virtex_up : public fpga
			{
			protected:
				/**
				 * @brief Constructs a Virtex UltraScale+ device.
				 */
				virtex_up(const std::string& name, uint32_t idcode, size_t num_brams);

				/**
				 * @brief Disposes a Virtex UltraScale device.
				 */
				virtual ~virtex_up() noexcept = 0;

			public:
				/**
				 * @brief Gets the number of block RAMs of this device.
				 */
				inline size_t num_brams(bram_category category) const override final
				{
					return (category == bram_category::ramb36) ? num_brams_ : 0u;
				}

				/**
				 * @brief Gets a block RAM (RAMB36E2) by its index.
				 */
				virtual const bram& bram_at(bram_category category, size_t index) const = 0;

			public:
				/**
				 * @brief Gets the Zynq-7 FPGA for a given IDCODE.
				 */
				static const virtex_up& get_by_idcode(uint32_t idcode);
			};

			//--------------------------------------------------------------------------------------
			/**
			 * @brief Detail implementation of a Virtex UltraScale+ variant
			 */
			template<uint32_t IdCode, size_t NumBrams>
			class virtex_up_variant : public virtex_up
			{
			private:
				/**
				 * @brief Block RAMs of this device. (RAMB36E2)
				 */
				const std::array<ramb36e2, NumBrams>& brams_;

			public:
				/**
				 * @brief Constructs a Zynq-7 variant
				 */
				virtex_up_variant(const std::string& name,
								  const std::array<ramb36e2, NumBrams>& brams)
					: virtex_up(name, IdCode, NumBrams), brams_(brams)
				{
				}

				/**
				 * @brief Disposes a Zynq-7 variant
				 */
				~virtex_up_variant() noexcept
				{
				}

			public:
				/**
				 * @brief Gets a block RAM by its index.
				 */
				const bram& bram_at(bram_category category, size_t index) const override
				{
					switch (category)
					{
					case bram_category::ramb36:
						return brams_.at(index);

					default:
						throw std::invalid_argument("unsupported block ram category");
					}
				}

			public:
				/**
				 * @brief Matches the IDCODE for this device variant.
				 */
				static bool match(uint32_t idcode)
				{
					return (IdCode == idcode);
				}
			};

			//--------------------------------------------------------------------------------------
			// XCVU9P
			//
			struct xcvu9p final
			{
				/**
				 * @brief Tries to match the IDCODE of the XC7Z010 device model.
				 */
				static bool match(uint32_t idcode);

				/**
				 * @brief Gets the XC7Z020 device model.
				 *
				 * @return A reference to the XC7Z010 device model.
				 */
				static const virtex_up& get();
			};
		}
	}
}

#endif // #ifndef UNBIT_XILINX_VUP_HPP_
