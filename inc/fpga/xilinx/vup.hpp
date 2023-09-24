/**
 * @file
 * @brief Common infrastructure for Xilinx Series-7 FPGAs
 *
 * @bug HIGHLY EXPERIMENTAL CODE (READ: HACK!)
 */
#ifndef FPGA_XILINX_VUP_HPP_
#define FPGA_XILINX_VUP_HPP_ 1

#include "common.hpp"
#include "bram.hpp"

#include <array>
#include <string>

namespace fpga
{
	namespace xilinx
	{
		/**
		 * @brief Common infrastructure for Xilinx Virtex UltraScale+ FPGAs
		 */
		namespace vup
		{
			// We reuse the Virtex-7 bitstream, bram, etc. classes (for now)
			using v7::bitstream;
			using v7::bram;
			using v7::bram_category;

			//--------------------------------------------------------------------------------------------------------------------
			/**
			 * @brief Description of a RAMB36E2 block RAM tile (as found on Virtex UltraScale+ devices)
			 */
			class ramb36e2 final : public bram
			{
			public:
				/**
				 * @brief Construct a RAMB36E1 block RAM tile.
				 */
				ramb36e2(unsigned x, unsigned y, size_t bitstream_offset);

				/**
				 * @brief Disposes a RAMB36E1 block RAM tile.
				 */
				~ramb36e2();

			public:
				const std::string& primitive() const override;

				size_t map_to_bitstream(size_t bit_addr, bool is_parity) const override;
			};
			
			//--------------------------------------------------------------------------------------------------------------------
			/**
			 * @brief Description of a Virtex UltraScale+ FPGA device
			 */
			class virtex_up
			{
			protected:
				/**
				 * @brief Name of this device.
				 */
				const std::string name_;

				/**
				 * @brief IDCODE of this FPGA model.
				 */
				const uint32_t idcode_;

				/**
				 * @brief Number of block RAMs (RAMB36E2) in this device.
				 */
				const size_t num_brams_;

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
				 * @brief Gets the name of this device.
				 */
				inline const std::string& name() const
				{
					return name_;
				}

				/**
				 * @brief Gets the IDCODE of this device.
				 */
				inline uint32_t idcode() const
				{
					return idcode_;
				}

				/**
				 * @brief Gets the number of block RAMs of this device.
				 */
				inline size_t num_brams(bram_category category) const
				{
					return (category == bram_category::ramb36) ? num_brams_ : 0u;
				}

				/**
				 * @brief Gets a block RAM (RAMB36E2) by its index.
				 */
				virtual const bram& bram_at(bram_category category, size_t index) const = 0;

				/**
				 * @brief Gets a block RAM (RAMB36E2) by its X/Y coordinate
				 */
				const bram& bram_by_loc(bram_category category, unsigned x, unsigned y) const;

			public:
				/**
				 * @brief Gets the Zynq-7 FPGA for a given IDCODE.
				 */
				static const virtex_up& get_by_idcode(uint32_t idcode);

			private:
				// Non-copyable
				virtex_up(virtex_up&) = delete;
				virtex_up& operator=(virtex_up&) = delete;
			};

			//--------------------------------------------------------------------------------------------------------------------
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
				virtex_up_variant(const std::string& name, const std::array<ramb36e2, NumBrams>& brams)
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
						throw std::invalid_argument("selected block ram category is not supported on this virtex ultrascale+ device");
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

			//--------------------------------------------------------------------------------------------------------------------
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

#endif // #ifndef FPGA_XILINX_VUP_HPP_
