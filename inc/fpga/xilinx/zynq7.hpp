/**
 * @file
 * @brief Common infrastructure for Xilinx Series-7 FPGAs
 */
#ifndef FPGA_XILINX_ZYNQ7_HPP_
#define FPGA_XILINX_ZYNQ7_HPP_ 1

#include "common.hpp"
#include "bram.hpp"

#include <array>
#include <string>

namespace fpga
{
	namespace xilinx
	{
		/**
		 * @brief Common infrastructure for Xilinx Series-7 FPGAs
		 */
		namespace v7
		{
			//--------------------------------------------------------------------------------------------------------------------
			/**
			 * @brief Description of a Zynq-7000 FPGA device
			 */
			class zynq7
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
				 * @brief Number of block RAMs (RAMB36E1) in this device.
				 */
				const size_t num_brams_;

			protected:
				/**
				* @brief Constructs a Zynq-7 device.
				 */
				zynq7(const std::string& name, uint32_t idcode, size_t num_brams);

				/**
				 * @brief Disposes a Zynq-7 device.
				 */
				virtual ~zynq7() noexcept = 0;

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
				inline size_t num_brams() const
				{
					return num_brams_;
				}

				/**
				 * @brief Gets a block RAM (RAMB36E1) by its index.
				 */
				virtual const bram& bram_at(size_t index) const = 0;

				/**
				 * @brief Gets a block RAM (RAMB36E1) by its X/Y coordinate
				 */
				const bram& bram_by_loc(unsigned x, unsigned y) const;

			public:
				/**
				 * @brief Gets the Zynq-7 FPGA for a given IDCODE.
				 */
				static const zynq7& get_by_idcode(uint32_t idcode);

			private:
				// Non-copyable
				zynq7(zynq7&) = delete;
				zynq7& operator=(zynq7&) = delete;
			};

			//--------------------------------------------------------------------------------------------------------------------
			/**
			 * @brief Detail implementation of a Zynq-7 variant
			 */
			template<uint32_t IdCode, size_t NumBrams>
			class zynq7_variant : public zynq7
			{
			private:
				/**
				 * @brief Block RAMs of this device.
				 */
				const std::array<ramb36e1, NumBrams>& brams_;

			public:
				/**
				 * @brief Constructs a Zynq-7 variant
				 */
				zynq7_variant(const std::string& name, const std::array<ramb36e1, NumBrams>& brams)
					: zynq7(name, IdCode, NumBrams), brams_(brams)
				{
				}

				/**
				 * @brief Disposes a Zynq-7 variant
				 */
				~zynq7_variant() noexcept
				{
				}

			public:
				/**
				 * @brief Gets a block RAM by its index.
				 */
				const bram& bram_at(size_t index) const override
				{
					return brams_.at(index);
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
			// XC7Z020
			//
			struct xc7z020 final
			{
				/**
				 * @brief Tries to match the IDCODE of the XC7Z020 device model.
				 */
				static bool match(uint32_t idcode);

				/**
				 * @brief Gets the XC7Z020 device model.
				 *
				 * @return A reference to the XC7Z020 device model.
				 */
				static const zynq7& get();
			};
		}
	}
}

#endif // #ifndef FPGA_XILINX_ZYNQ7_HPP_
