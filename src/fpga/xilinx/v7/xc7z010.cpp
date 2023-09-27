/**
 * @file
 * @brief Infrastructure for Xilinx Zynq-7000 FPGAs
 */
#include "fpga/xilinx/zynq7.hpp"
#include "fpga/xilinx/bram.hpp"

#include <array>

namespace unbit
{
	namespace xilinx
	{
		namespace v7
		{
			//--------------------------------------------------------------------------------------
			//
			// Implementation of the XC7Z010 FPGA.
			//

			// RAMB36E1 blocks
			static const std::array<ramb36e1, 60u> brams_36 =
			{
				ramb36e1 {  0u,  0u, 0x00EB0AC0u }, ramb36e1 {  0u,  1u, 0x00EB0C00u }, ramb36e1 {  0u,  2u, 0x00EB0D40u }, ramb36e1 {  0u,  3u, 0x00EB0E80u },
				ramb36e1 {  0u,  4u, 0x00EB0FC0u }, ramb36e1 {  0u,  5u, 0x00EB1120u }, ramb36e1 {  0u,  6u, 0x00EB1260u }, ramb36e1 {  0u,  7u, 0x00EB13A0u },
				ramb36e1 {  0u,  8u, 0x00EB14E0u }, ramb36e1 {  0u,  9u, 0x00EB1620u }, ramb36e1 {  0u, 10u, 0x00CB6180u }, ramb36e1 {  0u, 11u, 0x00CB62C0u },
				ramb36e1 {  0u, 12u, 0x00CB6400u }, ramb36e1 {  0u, 13u, 0x00CB6540u }, ramb36e1 {  0u, 14u, 0x00CB6680u }, ramb36e1 {  0u, 15u, 0x00CB67E0u },
				ramb36e1 {  0u, 16u, 0x00CB6920u }, ramb36e1 {  0u, 17u, 0x00CB6A60u }, ramb36e1 {  0u, 18u, 0x00CB6BA0u }, ramb36e1 {  0u, 19u, 0x00CB6CE0u },
				ramb36e1 {  1u,  0u, 0x00F15AC0u }, ramb36e1 {  1u,  1u, 0x00F15C00u }, ramb36e1 {  1u,  2u, 0x00F15D40u }, ramb36e1 {  1u,  3u, 0x00F15E80u },
				ramb36e1 {  1u,  4u, 0x00F15FC0u }, ramb36e1 {  1u,  5u, 0x00F16120u }, ramb36e1 {  1u,  6u, 0x00F16260u }, ramb36e1 {  1u,  7u, 0x00F163A0u },
				ramb36e1 {  1u,  8u, 0x00F164E0u }, ramb36e1 {  1u,  9u, 0x00F16620u }, ramb36e1 {  1u, 10u, 0x00D1B180u }, ramb36e1 {  1u, 11u, 0x00D1B2C0u },
				ramb36e1 {  1u, 12u, 0x00D1B400u }, ramb36e1 {  1u, 13u, 0x00D1B540u }, ramb36e1 {  1u, 14u, 0x00D1B680u }, ramb36e1 {  1u, 15u, 0x00D1B7E0u },
				ramb36e1 {  1u, 16u, 0x00D1B920u }, ramb36e1 {  1u, 17u, 0x00D1BA60u }, ramb36e1 {  1u, 18u, 0x00D1BBA0u }, ramb36e1 {  1u, 19u, 0x00D1BCE0u },
				ramb36e1 {  2u,  0u, 0x00F7AAC0u }, ramb36e1 {  2u,  1u, 0x00F7AC00u }, ramb36e1 {  2u,  2u, 0x00F7AD40u }, ramb36e1 {  2u,  3u, 0x00F7AE80u },
				ramb36e1 {  2u,  4u, 0x00F7AFC0u }, ramb36e1 {  2u,  5u, 0x00F7B120u }, ramb36e1 {  2u,  6u, 0x00F7B260u }, ramb36e1 {  2u,  7u, 0x00F7B3A0u },
				ramb36e1 {  2u,  8u, 0x00F7B4E0u }, ramb36e1 {  2u,  9u, 0x00F7B620u }, ramb36e1 {  2u, 10u, 0x00D80180u }, ramb36e1 {  2u, 11u, 0x00D802C0u },
				ramb36e1 {  2u, 12u, 0x00D80400u }, ramb36e1 {  2u, 13u, 0x00D80540u }, ramb36e1 {  2u, 14u, 0x00D80680u }, ramb36e1 {  2u, 15u, 0x00D807E0u },
				ramb36e1 {  2u, 16u, 0x00D80920u }, ramb36e1 {  2u, 17u, 0x00D80A60u }, ramb36e1 {  2u, 18u, 0x00D80BA0u }, ramb36e1 {  2u, 19u, 0x00D80CE0u },
			};

			typedef zynq7_variant<0x03722093u, 60u> xc7z010_variant;

			//--------------------------------------------------------------------------------------
			bool xc7z010::match(uint32_t idcode)
			{
				return xc7z010_variant::match(idcode);
			}

			//--------------------------------------------------------------------------------------
			const zynq7& xc7z010::get()
			{
				static const xc7z010_variant instance("xc7z010", brams_36);
				return instance;
			}
		}
	}
}
