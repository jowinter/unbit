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
			// Implementation of the XC7Z015 FPGA.
			//

			// RAMB36E1 blocks
			static const std::array<ramb36e1, 95u> brams_36 =
			{
				ramb36e1 {  0u,  0u, 0x0192EA40u }, ramb36e1 {  0u,  1u, 0x0192EB80u }, ramb36e1 {  0u,  2u, 0x0192ECC0u }, ramb36e1 {  0u,  3u, 0x0192EE00u },
				ramb36e1 {  0u,  4u, 0x0192EF40u }, ramb36e1 {  0u,  5u, 0x0192F0A0u }, ramb36e1 {  0u,  6u, 0x0192F1E0u }, ramb36e1 {  0u,  7u, 0x0192F320u },
				ramb36e1 {  0u,  8u, 0x0192F460u }, ramb36e1 {  0u,  9u, 0x0192F5A0u }, ramb36e1 {  1u,  0u, 0x01993A40u }, ramb36e1 {  1u,  1u, 0x01993B80u },
				ramb36e1 {  1u,  2u, 0x01993CC0u }, ramb36e1 {  1u,  3u, 0x01993E00u }, ramb36e1 {  1u,  4u, 0x01993F40u }, ramb36e1 {  1u,  5u, 0x019940A0u },
				ramb36e1 {  1u,  6u, 0x019941E0u }, ramb36e1 {  1u,  7u, 0x01994320u }, ramb36e1 {  1u,  8u, 0x01994460u }, ramb36e1 {  1u,  9u, 0x019945A0u },
				ramb36e1 {  2u,  0u, 0x019F8A40u }, ramb36e1 {  2u,  1u, 0x019F8B80u }, ramb36e1 {  2u,  2u, 0x019F8CC0u }, ramb36e1 {  2u,  3u, 0x019F8E00u },
				ramb36e1 {  2u,  4u, 0x019F8F40u }, ramb36e1 {  2u,  5u, 0x019F90A0u }, ramb36e1 {  2u,  6u, 0x019F91E0u }, ramb36e1 {  2u,  7u, 0x019F9320u },
				ramb36e1 {  2u,  8u, 0x019F9460u }, ramb36e1 {  2u,  9u, 0x019F95A0u }, ramb36e1 {  2u, 10u, 0x017FE100u }, ramb36e1 {  2u, 11u, 0x017FE240u },
				ramb36e1 {  2u, 12u, 0x017FE380u }, ramb36e1 {  2u, 13u, 0x017FE4C0u }, ramb36e1 {  2u, 14u, 0x017FE600u }, ramb36e1 {  2u, 15u, 0x017FE760u },
				ramb36e1 {  2u, 16u, 0x017FE8A0u }, ramb36e1 {  2u, 17u, 0x017FE9E0u }, ramb36e1 {  2u, 18u, 0x017FEB20u }, ramb36e1 {  2u, 19u, 0x017FEC60u },
				ramb36e1 {  2u, 20u, 0x016037C0u }, ramb36e1 {  2u, 21u, 0x01603900u }, ramb36e1 {  2u, 22u, 0x01603A40u }, ramb36e1 {  2u, 23u, 0x01603B80u },
				ramb36e1 {  2u, 24u, 0x01603CC0u }, ramb36e1 {  2u, 25u, 0x01603E20u }, ramb36e1 {  2u, 26u, 0x01603F60u }, ramb36e1 {  2u, 27u, 0x016040A0u },
				ramb36e1 {  2u, 28u, 0x016041E0u }, ramb36e1 {  2u, 29u, 0x01604320u }, ramb36e1 {  3u,  5u, 0x01A5E0A0u }, ramb36e1 {  3u,  6u, 0x01A5E1E0u },
				ramb36e1 {  3u,  7u, 0x01A5E320u }, ramb36e1 {  3u,  8u, 0x01A5E460u }, ramb36e1 {  3u,  9u, 0x01A5E5A0u }, ramb36e1 {  3u, 10u, 0x01863100u },
				ramb36e1 {  3u, 11u, 0x01863240u }, ramb36e1 {  3u, 12u, 0x01863380u }, ramb36e1 {  3u, 13u, 0x018634C0u }, ramb36e1 {  3u, 14u, 0x01863600u },
				ramb36e1 {  3u, 15u, 0x01863760u }, ramb36e1 {  3u, 16u, 0x018638A0u }, ramb36e1 {  3u, 17u, 0x018639E0u }, ramb36e1 {  3u, 18u, 0x01863B20u },
				ramb36e1 {  3u, 19u, 0x01863C60u }, ramb36e1 {  3u, 20u, 0x016687C0u }, ramb36e1 {  3u, 21u, 0x01668900u }, ramb36e1 {  3u, 22u, 0x01668A40u },
				ramb36e1 {  3u, 23u, 0x01668B80u }, ramb36e1 {  3u, 24u, 0x01668CC0u }, ramb36e1 {  3u, 25u, 0x01668E20u }, ramb36e1 {  3u, 26u, 0x01668F60u },
				ramb36e1 {  3u, 27u, 0x016690A0u }, ramb36e1 {  3u, 28u, 0x016691E0u }, ramb36e1 {  3u, 29u, 0x01669320u }, ramb36e1 {  4u, 10u, 0x018C8100u },
				ramb36e1 {  4u, 11u, 0x018C8240u }, ramb36e1 {  4u, 12u, 0x018C8380u }, ramb36e1 {  4u, 13u, 0x018C84C0u }, ramb36e1 {  4u, 14u, 0x018C8600u },
				ramb36e1 {  4u, 15u, 0x018C8760u }, ramb36e1 {  4u, 16u, 0x018C88A0u }, ramb36e1 {  4u, 17u, 0x018C89E0u }, ramb36e1 {  4u, 18u, 0x018C8B20u },
				ramb36e1 {  4u, 19u, 0x018C8C60u }, ramb36e1 {  4u, 20u, 0x016CD7C0u }, ramb36e1 {  4u, 21u, 0x016CD900u }, ramb36e1 {  4u, 22u, 0x016CDA40u },
				ramb36e1 {  4u, 23u, 0x016CDB80u }, ramb36e1 {  4u, 24u, 0x016CDCC0u }, ramb36e1 {  4u, 25u, 0x016CDE20u }, ramb36e1 {  4u, 26u, 0x016CDF60u },
				ramb36e1 {  4u, 27u, 0x016CE0A0u }, ramb36e1 {  4u, 28u, 0x016CE1E0u }, ramb36e1 {  4u, 29u, 0x016CE320u }
			};

			typedef zynq7_variant<0x0373B093u, 95u> xc7z015_variant;

			//--------------------------------------------------------------------------------------
			bool xc7z015::match(uint32_t idcode)
			{
				return xc7z015_variant::match(idcode);
			}

			//--------------------------------------------------------------------------------------
			const zynq7& xc7z015::get()
			{
				static const xc7z015_variant instance("xc7z015", brams_36);
				return instance;
			}
		}
	}
}
