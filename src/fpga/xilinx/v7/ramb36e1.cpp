/**
 * @file
 * @brief RAMB36E1 block RAM tile.
 */
#include "fpga/xilinx/bram.hpp"

namespace fpga
{
	namespace xilinx
	{
		//--------------------------------------------------------------------------------------------------------------------
		// RAMB36E1 tiles are (physically) organized as 1024 x (32+4) bits
		//
		// (RAMB18E1 macros map to the lower/upper half of a physical RAMB36E1 tile).
		//
		// The mapping between bits in the bitstream and data/parity bits in the RAM can be easily found by synthesizing an
		// FPGA design with just a single block RAM and generating a bitstream with logic location information. The mapping tables
		// in this file were inferred from the lofic location information produced by synthesis and implementation of such
		// a design (single_bram_top.v) for a Zynq XC7Z020 FPGA with Xilinx 2019.1 tools.
		//
		// Block RAM content is stored in a dedicated frame address range. Details on the frame address range can be found
		// in [Xilinx UG470; "Ch. 5 Configuration Details; Configuration Registers; Frame Address Register"]. From the logic
		// location information of our synthesis expriment we can infer that block RAM content is stored in contiguous (bit) chunks.
		//
		// Excerpt from the logic location information produced by bitgen for the single_bram_top.v design (on an XC7Z020 FPGA):
		//  > ...
		//  > Bit 29857216 0x00c20000      0 Block=RAMB36_X0Y0 Ram=B:BIT0     <== First configuration framen of this RAM
		//  > Bit 29857217 0x00c20000      1 Block=RAMB36_X0Y0 Ram=B:BIT64
		//  > Bit 29857218 0x00c20000      2 Block=RAMB36_X0Y0 Ram=B:BIT128
		//  > Bit 29857219 0x00c20000      3 Block=RAMB36_X0Y0 Ram=B:BIT192
		//  > Bit 29857220 0x00c20000      4 Block=RAMB36_X0Y0 Ram=B:BIT32
		//  > Bit 29857221 0x00c20000      5 Block=RAMB36_X0Y0 Ram=B:BIT96
		//  > Bit 29857279 0x00c20000     63 Block=RAMB36_X0Y0 Ram=B:BIT246
		//	> Bit 29857280 0x00c20000     64 Block=RAMB36_X0Y0 Ram=B:PARBIT0  <== Parity bits in first configuration frame.
		//	> Bit 29857281 0x00c20000     65 Block=RAMB36_X0Y0 Ram=B:PARBIT8
		//  > ...
		//  > Bit 29857535 0x00c20000    319 Block=RAMB36_X0Y0 Ram=B:BIT255   <== End of first configuration frame
		//  > Bit 29860448 0x00c20001      0 Block=RAMB36_X0Y0 Ram=B:BIT256   <== Start of second configuration frame of this RAM
		//  > Bit 29860449 0x00c20001      1 Block=RAMB36_X0Y0 Ram=B:BIT320
		//  > ...
		//  > Bit 29860511 0x00c20001     63 Block=RAMB36_X0Y0 Ram=B:BIT502
		//  > Bit 29860512 0x00c20001     64 Block=RAMB36_X0Y0 Ram=B:PARBIT32 <== Block of parity bits frame
		//  > Bit 29860513 0x00c20001     65 Block=RAMB36_X0Y0 Ram=B:PARBIT40
		//  > ...
		//  > Bit 30267999 0x00c2007f    319 Block=RAMB36_X0Y0 Ram=B:BIT32767 <== End of RAMB36_X0X0
		//  > Bit 29857536 0x00c20000    320 Block=RAMB36_X0Y1 Ram=B:BIT0     <== Start of RAMB36_X0Y1 (next RAM)
		//
		// The mapping of X/Y coordinates to the (start) byte offsets depends on the specific FPGA's geometry. The structure
		// inside the contiguous block for a RAM is identical for each RAM. We store the internal mapping of RAM bits here;
		// the start addresses of the config blocks are stored along with the specific FPGA type, and are communicated as
		// bitstream_offset to the constructor.
		//
		// NOTE: Uncompressed bit-stream seem to have their BRAM data swapped at 32-bit word-level. BRAM extract/insert functions
		// must take care of the word swapping (if needed).
		//
		//--------------------------------------------------------------------------------------------------------------------
		//
		// Further reverse engineering on the data-bit and parity-bit offsets shows a quite regular structure that can easily
		// be described using two small lookup tables (16 entries each), a constant (0xCA) and several shifts.
		//

		//
		// Configuration frames on Xilnx 7-series FPGAs are exctcly 101
		namespace v7
		{
			//--------------------------------------------------------------------------------------------------------------------
			/**
			 * @brief Maps from (relative) data-bit addresses to BRAM-relative bit offsets
			 *
			 * @note Reconstructed from the table based implementation (see git history for the original bit offset table).
			 */
			static uint32_t ramb36e1_map_data_bit(const uint32_t data_offset)
			{
				// Mapping table for the lower 4 bits of the result
				static const uint32_t groupL_table[] =
				{
					0x00u, 0x08u, 0x04u, 0x0Cu, 0x01u, 0x09u, 0x05u, 0x0Du,
					0x02u, 0x0Au, 0x06u, 0x0Eu, 0x03u, 0x0Bu, 0x07u, 0x0Fu
				};

				// Mapping table for higher 4 bits of thr result
				static const uint32_t groupH_table[] =
				{
					0x00u, 0x0Bu, 0x01u, 0x0Cu, 0x02u, 0x0Du, 0x03u, 0x0Eu,
					0x05u, 0x10u, 0x06u, 0x11u, 0x07u, 0x12u, 0x08u, 0x13u
				};

				if (data_offset >= 32768u)
					throw std::out_of_range("parity bit address to be mapped is out of bounds");

				// Block scale offset (for 256 entry blocks)
				static const uint32_t block_scale = 0xCAu;

				const uint32_t base_offset = (data_offset / 256u) * block_scale + groupH_table[data_offset & 0x0Fu];
				return (base_offset << 4u) + groupL_table[(data_offset >> 4) & 0x0Fu];
			}

			//--------------------------------------------------------------------------------------------------------------------
			/**
			 * @brief Maps from (relative) parity-bit addresses to BRAM-relative bit offsets
			 *
			 * @note Reconstructed from the table based implementation (see git history for the original bit offset table).
			 */
			static uint32_t ramb36e1_map_parity_bit(const uint32_t parity_offset)
			{
				// Mapping table for the lower 4 bits of the result
				static const uint32_t groupL_table[] =
				{
					0x00u, 0x08u, 0x04u, 0x0Cu, 0x01u, 0x09u, 0x05u, 0x0Du,
					0x02u, 0x0Au, 0x06u, 0x0Eu, 0x03u, 0x0Bu, 0x07u, 0x0Fu
				};

				// Mapping table for the higher 4 bits of the result.
				static const uint32_t groupP_table[] =
				{
					0x04u, 0x0Fu
				};

				if (parity_offset >= 4096u)
					throw std::out_of_range("parity bit address to be mapped is out of bounds");

				// Block scale offset (for 256 entry blocks)
				static const uint32_t block_scale = 0xCAu;

				const uint32_t base_offset = (parity_offset / 32u) * block_scale + groupP_table[parity_offset & 0x01u];
				return (base_offset << 4u) + groupL_table[(parity_offset >> 1) & 0x0Fu];
			}

			//--------------------------------------------------------------------------------------------------------------------
			ramb36e1::ramb36e1(unsigned x, unsigned y, size_t bitstream_offset, unsigned slr)
				: bram(x, y, 1024u, 32u, 4u, bram_category::ramb36, bitstream_offset, slr)
			{
			}

			//--------------------------------------------------------------------------------------------------------------------
			ramb36e1::~ramb36e1()
			{
			}

			//--------------------------------------------------------------------------------------------------------------------
			const std::string& ramb36e1::primitive() const
			{
				static const std::string primitive_name("RAMB36E1");
				return primitive_name;
			}

			//--------------------------------------------------------------------------------------------------------------------
			size_t ramb36e1::map_to_bitstream(size_t bit_addr, bool is_parity) const
			{
				if (is_parity)
				{
					// Map parity bit space	(note that data is swapped at 32-bit word level)
					return bitstream_offset_ + ramb36e1_map_parity_bit(bit_addr);
				}
				else
				{
					// Map data bit space (note that data is swapped at 32-bit word level)
					return bitstream_offset_ + ramb36e1_map_data_bit(bit_addr);
				}
			}
		}
	}
}
