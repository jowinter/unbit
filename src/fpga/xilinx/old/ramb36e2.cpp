/**
 * @file
 * @brief RAMB36E2 block RAM tile (Virtex UltraScale+)
 */
#include "unbit/fpga/old/xilinx/vup.hpp"
#include "unbit/fpga/old/xilinx/bram.hpp"

namespace unbit
{
	namespace old
	{
		namespace xilinx
		{
			//------------------------------------------------------------------------------------------
			// RAMB36E2 tiles are (physically) organized as 1024 x (32+4) bits
			//
			// The RAMB36E2 tiles work very similar to the Virtex-7 series tiles (e.g. the RAMB36E1
			// implementation in the v7 namespace).
			//
			// Mapping of bits in the bitstream can be determined (as for the Zynq-7 devices) by
			// synthesizing a design that uses all block RAMs and generating a bitstream with logic
			// location information. Mapping tables in this file were inferred in that way.
			//
			//------------------------------------------------------------------------------------------

			///! @brief Maps from (relative) data-bit addresses to BRAM-relative bit offsets
			static uint32_t ramb36e2_map_data_bit(const uint32_t data_offset)
			{
				// Mapping table for the lower 8 bits of the result
				static const uint8_t bit_table[] =
				{
					0x00u, 0x84u, 0x0Cu, 0x90u, 0x18u, 0x9Cu, 0x24u, 0xA8u,
					0x3Cu, 0xC0u, 0x48u, 0xCCu, 0x54u, 0xD8u, 0x60u, 0xE4u,
					0x06u, 0x8Au, 0x12u, 0x96u, 0x1Eu, 0xA2u, 0x2Au, 0xAEu,
					0x42u, 0xC6u, 0x4Eu, 0xD2u, 0x5Au, 0xDEu, 0x66u, 0xEAu,
					0x03u, 0x87u, 0x0Fu, 0x93u, 0x1Bu, 0x9Fu, 0x27u, 0xABu,
					0x3Fu, 0xC3u, 0x4Bu, 0xCFu, 0x57u, 0xDBu, 0x63u, 0xE7u,
					0x09u, 0x8Du, 0x15u, 0x99u, 0x21u, 0xA5u, 0x2Du, 0xB1u,
					0x45u, 0xC9u, 0x51u, 0xD5u, 0x5Du, 0xE1u, 0x69u, 0xEDu,
					0x02u, 0x86u, 0x0Eu, 0x92u, 0x1Au, 0x9Eu, 0x26u, 0xAAu,
					0x3Eu, 0xC2u, 0x4Au, 0xCEu, 0x56u, 0xDAu, 0x62u, 0xE6u,
					0x08u, 0x8Cu, 0x14u, 0x98u, 0x20u, 0xA4u, 0x2Cu, 0xB0u,
					0x44u, 0xC8u, 0x50u, 0xD4u, 0x5Cu, 0xE0u, 0x68u, 0xECu,
					0x05u, 0x89u, 0x11u, 0x95u, 0x1Du, 0xA1u, 0x29u, 0xADu,
					0x41u, 0xC5u, 0x4Du, 0xD1u, 0x59u, 0xDDu, 0x65u, 0xE9u,
					0x0Bu, 0x8Fu, 0x17u, 0x9Bu, 0x23u, 0xA7u, 0x2Fu, 0xB3u,
					0x47u, 0xCBu, 0x53u, 0xD7u, 0x5Fu, 0xE3u, 0x6Bu, 0xEFu
				};

				if (data_offset >= 32768u)
					throw std::out_of_range("data bit address to be mapped is out of bounds");

				// Block scale offset (for 128 entry blocks)
				static const uint32_t block_scale = 0xBA0u;

				return (data_offset >> 7u) * block_scale + bit_table[data_offset & 0x7Fu];
			}

			//------------------------------------------------------------------------------------------
			///! @brief Maps from (relative) parity-bit addresses to BRAM-relative bit offsets
			static uint32_t ramb36e2_map_parity_bit(const uint32_t parity_offset)
			{
				// Mapping table for the lower 4 bits of the result
				static const uint8_t bit_table[] =
				{
					0x30u, 0xB4u, 0x36u, 0xBAu, 0x33u, 0xB7u, 0x39u, 0xBDu,
					0x32u, 0xB6u, 0x38u, 0xBCu, 0x35u, 0xB9u, 0x3Bu, 0xBFu
				};

				if (parity_offset >= 4096u)
					throw std::out_of_range("parity bit address to be mapped is out of bounds");

				// Block scale offset (for 16 entry blocks)
				static const uint32_t block_scale = 0xBA0u;

				return (parity_offset >> 4u) * block_scale + bit_table[parity_offset & 0xF];
			}

			//------------------------------------------------------------------------------------------
			ramb36e2::ramb36e2(unsigned x, unsigned y, size_t bitstream_offset, unsigned slr)
				: bram(x, y, 1024u, 32u, 4u, bram_category::ramb36, bitstream_offset, slr)
			{
			}

			//------------------------------------------------------------------------------------------
			ramb36e2::~ramb36e2()
			{
			}

			//------------------------------------------------------------------------------------------
			const std::string& ramb36e2::primitive() const
			{
				static const std::string primitive_name("RAMB36E2");
				return primitive_name;
			}

			//------------------------------------------------------------------------------------------
			size_t ramb36e2::map_to_bitstream(size_t bit_addr, bool is_parity) const
			{
				if (is_parity)
				{
					// Map parity bit space	(note that data is swapped at 32-bit word level)
					return bitstream_offset_ + ramb36e2_map_parity_bit(bit_addr);
				}
				else
				{
					// Map data bit space (note that data is swapped at 32-bit word level)
					return bitstream_offset_ + ramb36e2_map_data_bit(bit_addr);
				}
			}
		}	
	}
}
