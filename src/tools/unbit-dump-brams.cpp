/**
 * @file
 * @brief Proof-of-concept tool to dump the contents of all block RAMs in a Xilinx FPGA (as INIT/INITP strings).
 */

#include "fpga/xilinx/bitstream.hpp"
#include "fpga/xilinx/bram.hpp"
#include "fpga/xilinx/fpga.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>

using unbit::xilinx::bitstream;
using unbit::xilinx::bram;
using unbit::xilinx::bram_category;

//---------------------------------------------------------------------------------------------------------------------
static void dump_ram_data(const bitstream& bs, const bram& ram, bool is_parity)
{
	const size_t line_width = 32u;
	const std::vector<uint8_t> data = ram.extract(bs, is_parity);

	for (size_t line_offset = 0u; line_offset < data.size(); line_offset += line_width)
	{
		uint8_t line_buf[line_width];

		// Fill the line buffer (missing bytes are silently buffer with zero
		for (size_t i = 0u; i < line_width; ++i)
		{
			line_buf[i] = (line_offset + i) < data.size() ? data[line_offset + i] : 0u;
		}

		std::cout << (is_parity ? "INITP_" : "INIT_")
				  << std::hex << std::setw(2) << std::setfill('0') << (line_offset / line_width)
				  << ": " << std::dec << (line_width * 8u) << "'h";

		for (size_t i = 0u; i < line_width; ++i)
		{
			// Xilinx INIT/INITP strings use a reversed order
			std::cout << std::hex << std::setw(2) << std::setfill('0')
					  << static_cast<unsigned>(line_buf[line_width - i - 1u]);
		}

		std::cout << std::endl;
	}
}

//---------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "usage: " << argv[0u] << " <bitstream>" << std::endl
					  << std::endl
					  << "note: dumping of raw readback files has been removed from this tool"
					  << std::endl
					  << "extraction of bram content is possible by using the bram substitution"
					  << std::endl
					  << "tool to inject the readback data into a fresh bitstream, followed by use"
					  << std::endl
					  << "of this tool for extraction of bram content in textual form."
					  << std::endl << std::endl;
			return EXIT_FAILURE;
		}

		// Check for an explicitly given IDCODE
		const bitstream bs = bitstream::load_bitstream(argv[1u]);
		std::cout << "// IDCODE: 0x" << std::hex << bs.idcode() << std::dec << std::endl;

		const auto& fpga = unbit::xilinx::fpga_by_idcode(bs.idcode());
		std::cout << "// FPGA: " << fpga.name() << std::endl;

		// Provide some info on the geometry of the FPGA
		const auto num_slrs = bs.slrs().size();
		for (size_t i = 0u; i < num_slrs; ++i)
		{
			const auto& slr = bs.slr(i);

			std::cout << "// BITSTREAM: SLR" << std::dec << i;

			if (slr.frame_data_size > 0)
			{
				const auto frame_end = slr.frame_data_offset + slr.frame_data_size - 1u;

				std::cout << " frame@0x" << std::hex
						  << std::setw(8) << std::setfill('0')
						  << slr.frame_data_offset
						  << "..0x" << std::setw(8) << std::setfill('0')
						  << frame_end
						  << " (" << std::dec
						  << slr.frame_data_size << " bytes)";
			}


			if (slr.sync_offset != 0xFFFFFFFFu)
			{
				std::cout << " sync@0x" << std::hex << std::setw(8) << std::setfill('0')
						  << slr.sync_offset;
			}

			std::cout << std::dec << std::endl;
		}

		std::cout << std::endl;

		// And here come the RAMs
		for (size_t i = 0u; i < fpga.num_brams(bram_category::ramb36); ++i)
		{
			const bram& ram = fpga.bram_at(bram_category::ramb36, i);

			// Determine the storage offset of the RAM block in the bitstream file.
			const auto& slr = bs.slr(ram.slr());
			const size_t ram_bit_offset     = ram.bitstream_offset();
			const size_t ram_storage_offset = slr.frame_data_offset + (ram_bit_offset / 8u);


			std::cout << "//" << std::endl
					  << "// " << ram << std::endl
					  << "//" << std::endl
					  << "// SLR" << std::dec << ram.slr() << "+0x"
					  << std::hex << std::setw(8) << std::setfill('0') << ram_bit_offset
					  << " storage@0x"
					  << std::setw(8) << std::setfill('0') << ram_storage_offset
					  << std::dec << std::endl << std::endl;

			// Dump the RAM word data
			dump_ram_data(bs, ram, false);
			std::cout << std::endl;

			// And the parity data
			dump_ram_data(bs, ram, true);
			std::cout << std::endl;
		}

		return EXIT_SUCCESS;
	}
	catch (std::exception& e)
	{
		std::cerr << std::endl << "error: unhandled exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
