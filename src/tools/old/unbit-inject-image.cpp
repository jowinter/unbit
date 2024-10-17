/**
 * @file
 * @brief Proof-of-concept tool to inject a (processor) image into block RAMs of a Xilinx FPGA.
 */

#include "fpga/old/xilinx/bitstream.hpp"
#include "fpga/old/xilinx/bram.hpp"
#include "fpga/old/xilinx/mmi.hpp"
#include "fpga/old/xilinx/fpga.hpp"

#include "xml/xml.hpp"
#include "ihex/ihex.hpp"

#include <iostream>

using unbit::old::xilinx::bitstream;
using unbit::old::xilinx::bram;
using unbit::old::xilinx::bram_category;
using unbit::old::xilinx::fpga;
using unbit::old::xilinx::fpga_by_idcode;
using unbit::old::xilinx::mmi::memory_map;
using unbit::old::xilinx::mmi::memory_region;

using unbit::xml::xml_parser_guard;

//---------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	xml_parser_guard parser_guard;

	try
	{
		if (argc != 6u)
		{
			std::cerr << "usage: " << argv[0u] << " <result> <bitstream> <mmi> <instance> <ihex>" << std::endl
					  << std::endl;
			return EXIT_FAILURE;
		}

		bitstream bs = bitstream::load_bitstream(argv[2u], 0xFFFFFFFFu, true);
		const fpga& fpga = fpga_by_idcode(bs.idcode());
		const auto mmi = memory_map::load(argv[3u], argv[4u]);

		std::cout << "updating brams from intel hex image ..." << std::flush;

		size_t total_load_size = 0u;

		unbit::ihex::load(argv[5u], [&] (uint32_t address, const std::vector<uint8_t>& data)
		{
			// Inject bytes into the working copy of the bitstream
			for (size_t i = 0u; i < data.size(); ++i)
			{
				mmi->write_byte(bs, fpga, address + i, data[i]);
			}

			// Account total number of bytes that have been loaded
			total_load_size += data.size();
		});

		std::cout << total_load_size << " bytes loaded" << std::endl;

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
