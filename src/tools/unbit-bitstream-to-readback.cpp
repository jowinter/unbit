/**
 * @file
 * @brief Performs a (virtual) readback of a Xilinx bitstream.
 */

#include "fpga/xilinx/bitstream.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>

using unbit::xilinx::bitstream;

//--------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "usage: " << argv[0u] << " <result> <bitstream/rbb>" << std::endl
					  << std::endl
					  << "Performs a virtual readback of a bitstream. The output file is a "
					  << "simulation of the readback data file produced by the 'readback_hw_device'"
					  << std::endl
					  << "readback command. The input file can be an uncompressed bitstream (.bit)."
					  << std::endl
					  << "The virtual readback is performed by concatenating frame data (FDRI/FDRO)"
					  << std::endl
					  << "commands in the bitstream being processed."
					  << std::endl << std::endl;
			return EXIT_FAILURE;
		}

		// Load the incoming bitstream (we accept normal config bitstreams and 'readback' bitstreams)
		std::cout << "loading bitstream ..." << std::flush;
		const bitstream source = bitstream::load_bitstream(argv[2], 0xFFFFFFFFu, true);
		std::cout << "done" << std::endl;

		// And write the (simulated) readback
		std::cout << "writing simulated readback file ..." << std::flush;
		bitstream::save_as_readback(argv[1], source);
		std::cout << "done" << std::endl;

		return EXIT_SUCCESS;
	}
	catch (std::exception& e)
	{
		std::cerr << std::endl << "error: unhandled exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
