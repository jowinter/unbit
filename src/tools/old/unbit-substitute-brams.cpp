/**
 * @file
 * @brief Proof-of-concept tool to replace BRAM initialization data in a bitstream.
 */

#include "unbit/fpga/old/xilinx/bitstream.hpp"
#include "unbit/fpga/old/xilinx/bram.hpp"
#include "unbit/fpga/old/xilinx/fpga.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>

using unbit::old::xilinx::bitstream;
using unbit::old::xilinx::bram;
using unbit::old::xilinx::bram_category;

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
		bitstream bs = bitstream::load_bitstream(argv[2u]);
		
		const auto& fpga = unbit::old::xilinx::fpga_by_idcode(bs.idcode());
		std::cout << "fpga: " << fpga.name() << std::endl;

		// Load the source RAMs (with inference of bitstream properties from the given bitstream)
		const bitstream brams = bitstream::load_raw(argv[3u], bs);

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
		//
		// HACK: We ought to do this properly ... for now the unbit-strip-crc-checks tool can
		// be used to neutralize all CRC checks.
		std::cout << "warning: crc checks in the result bitstream (if present) need to be fixed up." << std::endl
			  << "warning: the unbit-strip-crc-checks tool can be used to strip all (sic!) crc" << std::endl
			  << "warning: check commands from the result (and/or source) bitstream." << std::endl;

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
