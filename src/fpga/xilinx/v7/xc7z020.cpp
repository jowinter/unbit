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
			// Implementation of the XC7Z020 FPGA.			
			//
			static const std::array<ramb36e1, 140u> brams =
			{
					ramb36e1 { 0,  0, 0x01C795C0}, ramb36e1 { 0,  1, 0x01C79700}, ramb36e1 { 0,  2, 0x01C79840}, ramb36e1 { 0,  3, 0x01C79980},
					ramb36e1 { 0,  4, 0x01C79AC0}, ramb36e1 { 0,  5, 0x01C79C20}, ramb36e1 { 0,  6, 0x01C79D60}, ramb36e1 { 4,  0, 0x01E0D5C0},
					ramb36e1 { 0,  7, 0x01C79EA0}, ramb36e1 { 4,  1, 0x01E0D700}, ramb36e1 { 0,  8, 0x01C79FE0}, ramb36e1 { 4,  2, 0x01E0D840},
					ramb36e1 { 0,  9, 0x01C7A120}, ramb36e1 { 4,  3, 0x01E0D980}, ramb36e1 { 4,  4, 0x01E0DAC0}, ramb36e1 { 4,  5, 0x01E0DC20},
					ramb36e1 { 4,  6, 0x01E0DD60}, ramb36e1 { 4,  7, 0x01E0DEA0}, ramb36e1 { 4,  8, 0x01E0DFE0}, ramb36e1 { 1,  0, 0x01CDE5C0},
					ramb36e1 { 4,  9, 0x01E0E120}, ramb36e1 { 1,  1, 0x01CDE700}, ramb36e1 { 4, 10, 0x01BADC80}, ramb36e1 { 1,  2, 0x01CDE840},
					ramb36e1 { 4, 11, 0x01BADDC0}, ramb36e1 { 1,  3, 0x01CDE980}, ramb36e1 { 4, 12, 0x01BADF00}, ramb36e1 { 1,  4, 0x01CDEAC0},
					ramb36e1 { 4, 13, 0x01BAE040}, ramb36e1 { 1,  5, 0x01CDEC20}, ramb36e1 { 4, 14, 0x01BAE180}, ramb36e1 { 1,  6, 0x01CDED60},
					ramb36e1 { 4, 15, 0x01BAE2E0}, ramb36e1 { 1,  7, 0x01CDEEA0}, ramb36e1 { 4, 16, 0x01BAE420}, ramb36e1 { 1,  8, 0x01CDEFE0},
					ramb36e1 { 4, 17, 0x01BAE560}, ramb36e1 { 1,  9, 0x01CDF120}, ramb36e1 { 4, 18, 0x01BAE6A0}, ramb36e1 { 4, 19, 0x01BAE7E0},
					ramb36e1 { 4, 20, 0x0194E340}, ramb36e1 { 2,  0, 0x01D435C0}, ramb36e1 { 4, 21, 0x0194E480}, ramb36e1 { 2,  1, 0x01D43700},
					ramb36e1 { 4, 22, 0x0194E5C0}, ramb36e1 { 2,  2, 0x01D43840}, ramb36e1 { 4, 23, 0x0194E700}, ramb36e1 { 2,  3, 0x01D43980},
					ramb36e1 { 4, 24, 0x0194E840}, ramb36e1 { 2,  4, 0x01D43AC0}, ramb36e1 { 4, 25, 0x0194E9A0}, ramb36e1 { 2,  5, 0x01D43C20},
					ramb36e1 { 4, 26, 0x0194EAE0}, ramb36e1 { 2,  6, 0x01D43D60}, ramb36e1 { 4, 27, 0x0194EC20}, ramb36e1 { 2,  7, 0x01D43EA0},
					ramb36e1 { 4, 28, 0x0194ED60}, ramb36e1 { 2,  8, 0x01D43FE0}, ramb36e1 { 4, 29, 0x0194EEA0}, ramb36e1 { 2,  9, 0x01D44120},
					ramb36e1 { 2, 10, 0x01AE3C80}, ramb36e1 { 2, 11, 0x01AE3DC0}, ramb36e1 { 2, 12, 0x01AE3F00}, ramb36e1 { 2, 13, 0x01AE4040},
					ramb36e1 { 2, 14, 0x01AE4180}, ramb36e1 { 2, 15, 0x01AE42E0}, ramb36e1 { 5,  0, 0x01E725C0}, ramb36e1 { 2, 16, 0x01AE4420},
					ramb36e1 { 5,  1, 0x01E72700}, ramb36e1 { 2, 17, 0x01AE4560}, ramb36e1 { 5,  2, 0x01E72840}, ramb36e1 { 2, 18, 0x01AE46A0},
					ramb36e1 { 5,  3, 0x01E72980}, ramb36e1 { 2, 19, 0x01AE47E0}, ramb36e1 { 5,  4, 0x01E72AC0}, ramb36e1 { 5,  5, 0x01E72C20},
					ramb36e1 { 2, 20, 0x01884340}, ramb36e1 { 5,  6, 0x01E72D60}, ramb36e1 { 2, 21, 0x01884480}, ramb36e1 { 5,  7, 0x01E72EA0},
					ramb36e1 { 2, 22, 0x018845C0}, ramb36e1 { 5,  8, 0x01E72FE0}, ramb36e1 { 2, 23, 0x01884700}, ramb36e1 { 5,  9, 0x01E73120},
					ramb36e1 { 2, 24, 0x01884840}, ramb36e1 { 5, 10, 0x01C12C80}, ramb36e1 { 2, 25, 0x018849A0}, ramb36e1 { 5, 11, 0x01C12DC0},
					ramb36e1 { 2, 26, 0x01884AE0}, ramb36e1 { 5, 12, 0x01C12F00}, ramb36e1 { 2, 27, 0x01884C20}, ramb36e1 { 5, 13, 0x01C13040},
					ramb36e1 { 2, 28, 0x01884D60}, ramb36e1 { 5, 14, 0x01C13180}, ramb36e1 { 2, 29, 0x01884EA0}, ramb36e1 { 5, 15, 0x01C132E0},
					ramb36e1 { 5, 16, 0x01C13420}, ramb36e1 { 5, 17, 0x01C13560}, ramb36e1 { 5, 18, 0x01C136A0}, ramb36e1 { 5, 19, 0x01C137E0},
					ramb36e1 { 3,  0, 0x01DA85C0}, ramb36e1 { 5, 20, 0x019B3340}, ramb36e1 { 3,  1, 0x01DA8700}, ramb36e1 { 5, 21, 0x019B3480},
					ramb36e1 { 3,  2, 0x01DA8840}, ramb36e1 { 5, 22, 0x019B35C0}, ramb36e1 { 3,  3, 0x01DA8980}, ramb36e1 { 5, 23, 0x019B3700},
					ramb36e1 { 3,  4, 0x01DA8AC0}, ramb36e1 { 5, 24, 0x019B3840}, ramb36e1 { 3,  5, 0x01DA8C20}, ramb36e1 { 5, 25, 0x019B39A0},
					ramb36e1 { 3,  6, 0x01DA8D60}, ramb36e1 { 5, 26, 0x019B3AE0}, ramb36e1 { 3,  7, 0x01DA8EA0}, ramb36e1 { 5, 27, 0x019B3C20},
					ramb36e1 { 3,  8, 0x01DA8FE0}, ramb36e1 { 5, 28, 0x019B3D60}, ramb36e1 { 3,  9, 0x01DA9120}, ramb36e1 { 5, 29, 0x019B3EA0},
					ramb36e1 { 3, 10, 0x01B48C80}, ramb36e1 { 3, 11, 0x01B48DC0}, ramb36e1 { 3, 12, 0x01B48F00}, ramb36e1 { 3, 13, 0x01B49040},
					ramb36e1 { 3, 14, 0x01B49180}, ramb36e1 { 3, 15, 0x01B492E0}, ramb36e1 { 3, 16, 0x01B49420}, ramb36e1 { 3, 17, 0x01B49560},
					ramb36e1 { 3, 18, 0x01B496A0}, ramb36e1 { 3, 19, 0x01B497E0}, ramb36e1 { 3, 20, 0x018E9340}, ramb36e1 { 3, 21, 0x018E9480},
					ramb36e1 { 3, 22, 0x018E95C0}, ramb36e1 { 3, 23, 0x018E9700}, ramb36e1 { 3, 24, 0x018E9840}, ramb36e1 { 3, 25, 0x018E99A0},
					ramb36e1 { 3, 26, 0x018E9AE0}, ramb36e1 { 3, 27, 0x018E9C20}, ramb36e1 { 3, 28, 0x018E9D60}, ramb36e1 { 3, 29, 0x018E9EA0}
			};

			typedef zynq7_variant<0x03727093u, 140> xc7z020_variant;

			//--------------------------------------------------------------------------------------------------------------------			
			bool xc7z020::match(uint32_t idcode)
			{
				return xc7z020_variant::match(idcode);
			}

			//--------------------------------------------------------------------------------------------------------------------			
			const zynq7& xc7z020::get()
			{
				static const xc7z020_variant instance("xc7z020", brams);
				return instance;
			}			
		};
	}
}
