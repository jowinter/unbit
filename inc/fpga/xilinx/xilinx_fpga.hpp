/**
 * @file
 * @brief Common infrastructure for Xilinx Series-7 (and alike) FPGAs
 */
#ifndef FPGA_XILINX_FPGA_HPP_
#define FPGA_XILINX_FPGA_HPP_ 1

#include "common.hpp"
#include "bram.hpp"

#include <array>
#include <string>

namespace fpga
{
	namespace xilinx
	{
		using v7::bram;
		using v7::bram_category;

		/**
		 * @brief Common baseline for Xilinx Virtex-7 FPGAs (and lookalikes)
		 */
		class xilinx_fpga
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
			xilinx_fpga(const std::string& name, uint32_t idcode, size_t num_brams);

			/**
			 * @brief Disposes a Zynq-7 device.
			 */
			virtual ~xilinx_fpga() noexcept = 0;

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
			virtual size_t num_brams(bram_category category) const = 0;

			/**
			 * @brief Gets a block RAM by its index.
			 */
			virtual const bram& bram_at(bram_category category, size_t index) const = 0;

			/**
			 * @brief Gets a block RAM by its X/Y coordinate
			 */
			const bram& bram_by_loc(bram_category category, unsigned x, unsigned y) const;

		private:
			// Non-copyable
			xilinx_fpga(xilinx_fpga&) = delete;
			xilinx_fpga& operator=(xilinx_fpga&) = delete;
		};

		///! @todo Currently a hack (refctoring in progress)
		extern const xilinx_fpga& xilinx_fpga_by_idcode(const uint32_t idcode);
	}
}

#endif // FPGA_XILINX_FPGA_HPP_
