/**
 * @file
 * @brief Common infrastructure for Xilinx FPGAs
 */
#include "unbit/fpga/xilinx/device.hpp"

#include <stdexcept>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			//------------------------------------------------------------------------------------------
            device::device(const std::string& name, std::size_t words_per_frame, std::size_t frames_per_device,
                std::size_t slrs_per_device)
            : name_(name), words_per_frame_(words_per_frame), frames_per_device_(frames_per_device),
                slrs_per_device_(slrs_per_device)
            {
            }

            //------------------------------------------------------------------------------------------
            device::~device()
            {
            }

            //------------------------------------------------------------------------------------------
            device::linear_frame_addr_t device::phys_to_linear(uint32_t far) const
            {
                ///! @todo fixme: implement device::phys_to_linear()
                throw std::logic_error("fixme: device::phys_to_linear() is not implemented");
            }

            //------------------------------------------------------------------------------------------
            uint32_t device::linear_to_phys(uint32_t addr) const
            {
                ///! @todo fixme: implement device::linear_to_phys()
                throw std::logic_error("fixme: device::linear_to_phys() is not implemented");
            }
        }
    }
}