/**
 * @file
 * @brief Proof-of-concept tool to strip all CRC check commands from a bitstream.
 */

#include "fpga/old/xilinx/bitstream.hpp"
#include "fpga/old/xilinx/fpga.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>

using unbit::old::xilinx::bitstream;

//---------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "usage: " << argv[0u] << " <result> <bitstream>" << std::endl
				  << std::endl
				  << "Strips all CRC check commands from a bitstream (while leaving the rest of the bitstream intact)" << std::endl
				  << std::endl << std::endl
				  << std::endl;
			return EXIT_FAILURE;
		}

		// Load the bitstream to be updated
		bitstream bs = bitstream::load_bitstream(argv[2u]);

		const auto& fpga = unbit::old::xilinx::fpga_by_idcode(bs.idcode());
		std::cout << "fpga: " << fpga.name() << std::endl;

		std::cout << "stripping crc checks ..." << std::flush;
		bs.strip_crc_checks();
		std::cout << "done" << std::endl;

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
