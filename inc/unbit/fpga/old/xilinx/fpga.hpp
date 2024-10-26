/**
 * @file
 * @brief Common infrastructure for Xilinx Series-7 (and alike) FPGAs
 */
#ifndef UNBIT_OLD_XILINX_FPGA_HPP_
#define UNBIT_OLD_XILINX_FPGA_HPP_ 1

#include "common.hpp"
#include "bram.hpp"

#include <array>
#include <string>

namespace unbit
{
	namespace old
	{
		namespace xilinx
		{
			/**
			* @brief Common baseline for Xilinx Virtex-7 FPGAs (and lookalikes)
			*/
			class fpga
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

				/**
				* @brief Size of a configuration frame
				*/
				const size_t frame_size_;

				/**
				* @brief Size of the "pipeline" padding at the start of readback.
				*/
				const size_t readback_offset_;

				/**
				* @brief Size of the padding in front of the frame data in a raw readback stream.
				*/
				const size_t front_padding_;

				/**
				* @brief Size of the padding after the frame data in a raw readback stream.
				*
				* @note This value includes the size required by the back sync words.
				*/
				const size_t back_padding_;

				/**
				* @brief Number of extra sync words after the back padding in a raw readback stream.
				*/
				const size_t back_sync_words_;

			protected:
				/**
				* @brief Constructs a generic device.
				*/
				fpga(const std::string& name, uint32_t idcode, size_t num_brams, size_t frame_size,
					size_t readback_offset, size_t front_padding, size_t back_padding,
					size_t back_sync_words);

				/**
				* @brief Disposes a Zynq-7 device.
				*/
				virtual ~fpga() noexcept = 0;

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
				* @brief Gets the size of a single configuration frame (in bytes)
				*/
				inline uint32_t frame_size() const
				{
					return frame_size_;
				}

				/**
				* @brief Gets the number of leading "extra" bytes in front of raw readback data in
				*   the payload of an FDRO stream.
				*/
				inline uint32_t readback_offset() const
				{
					return readback_offset_;
				}

				/**
				* @brief Gets the size of extra padding in front of frame data in a raw readback
				*   stream.
				*/
				inline uint32_t front_padding() const
				{
					return front_padding_;
				}

				/**
				* @brief Gets the size of extra padding after frame data in a raw readback
				*   stream (excluding the back sync).
				*/
				inline uint32_t back_padding() const
				{
					return back_padding_;
				}

				/**
				* @brief Gets the number of extra sync words after the back padding in a raw readback
				*   stream.
				*/
				inline uint32_t back_sync_words() const
				{
					return back_sync_words_;
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
				fpga(fpga&) = delete;
				fpga& operator=(fpga&) = delete;
			};

			//------------------------------------------------------------------------------------------
			/**
			* @brief Gets a known Xlinx FPGA by its IDCODE.
			*
			* @param[in] idcode is the IDCODE of the FPGA.
			*/
			extern const fpga& fpga_by_idcode(const uint32_t idcode);
		}
	}
}

#endif // UNBIT_OLD_XILINX_FPGA_HPP_
