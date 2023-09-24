/**
 * @file
 * @brief Proof-of-concept tool to replace BRAM initialization data in a bitstream.
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
int main(int argc, char *argv[])
{
	try
	{
		if (argc != 4)
		{
			std::cerr << "usage: " << argv[0u] << " <result> <bitstream> <readback-file>" << std::endl
				<< std::endl
				<< "Substitutes initialization data of BRAM blocks in a given <bitstream> by BRAM content obtained" << std::endl
				<< "obtained from FPGA readback (read_back_hw_device -bin_file). The resulting bitstream, with substituted"  << std::endl
				<< "BRAMs is written to <result> and can be used to configure FPGAs (note that this tool currently does not" << std::endl
				<< "update CRC values)" << std::endl << std::endl
				<< std::endl;
			return EXIT_FAILURE;
		}

		// Load the bitstream to be updated
		bitstream bs = bitstream::load(argv[2u], bitstream::format::bit);
		const xilinx_fpga& fpga = xilinx_fpga_by_idcode(bs.idcode());
		std::cout << "fpga: " << fpga.name() << std::endl;

		// Load the source RAMs
		const bitstream brams = bitstream::load(argv[3u], bitstream::format::raw, bs.idcode());

		std::cout << "substituting brams " << std::flush;

		for (size_t i = 0u; i < fpga.num_brams(bram_category::ramb36); ++i)
		{
			// Straightforward extract/inject (via copy)
			const bram& ram = fpga.bram_at(bram_category::ramb36, i);

			// Data
			ram.inject(bs, false, ram.extract(brams, false));

			// Parity
			ram.inject(bs, true, ram.extract(brams, true));

			std::cout << "." << std::flush;
		}

		std::cout << std::endl;

		// Need to fixup the CRC record (for now we simply kill the CRC command)
		// HACK: We ought to do this properly ...
		std::cout << "stripping crc checks (fixme!)" << std::endl;
		bs.update_crc();


		// And store the output
		std::cout << "writing result bitstream ..." << std::flush;
		bitstream::save(argv[1u], bs);
		std::cout << "done" << std::endl;
		return EXIT_SUCCESS;
	}
	catch (std::exception& e)
	{
		std::cerr << std::endl << "error: unhandled exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
