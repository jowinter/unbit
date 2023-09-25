/**
 * @file
 * @brief Proof-of-concept tool to dump the contents of all block RAMs in a Xilinx FPGA (as INIT/INITP strings).
 */

#include "fpga/xilinx/bitstream.hpp"
#include "fpga/xilinx/bram.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>

using fpga::xilinx::v7::bitstream;
using fpga::xilinx::v7::bram;
using fpga::xilinx::v7::bram_category;

#include "fpga_family.hpp"

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
		if ((argc < 2) || (argc > 3))
		{
			std::cerr << "usage: " << argv[0u] << " <bitstream> [<fpga>]" << std::endl
				  << std::endl << std::endl;
			return EXIT_FAILURE;
		}

		// Check for an explicitly given IDCODE
		uint32_t expected_idcode;
		bitstream::format fmt;

		{
			if (argc > 2)
			{
				expected_idcode = std::stoul(argv[2], nullptr, 0);
				fmt = bitstream::format::raw;
				std::cout << "// Raw frames / Readback data @ " << argv[1] << std::endl;
			}
			else
			{
				expected_idcode = 0xFFFFFFFFu;
				fmt = bitstream::format::bit;
				std::cout << "// Bitstream data @ " << argv[1] << std::endl;
			}
		}

		const bitstream bs = bitstream::load(argv[1u], fmt, expected_idcode);
		std::cout << "// IDCODE: 0x" << std::hex << bs.idcode() << std::dec << std::endl;

		const xilinx_fpga& fpga = xilinx_fpga_by_idcode(bs.idcode());
		std::cout << "// FPGA: " << fpga.name() << std::endl << std::endl;

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
				  << " (storage @ 0x"
				  << std::hex << std::setw(8) << std::setfill('0') << ram_storage_offset
				  << ")" << std::endl << std::endl;

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
