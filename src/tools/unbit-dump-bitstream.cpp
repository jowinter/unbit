/**
 * @file
 * @brief Proof-of-concept tool to dump the structure of a Xilinx 7-series bitstream
 */

#include "fpga/xilinx/bitstream.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>

using fpga::xilinx::v7::bitstream;

//---------------------------------------------------------------------------------------------------------------------
static bool dump_packet(const bitstream::packet& pkt)
{
	std::cout << "[" << std::hex << std::setw(8u) << std::setfill('0') << pkt.offset
		<< "] 0x" << std::setw(8u) << std::setfill('0') << pkt.hdr << std::endl;

	// Append a hexdump of the payload
	for (auto pos = pkt.payload_start; pos != pkt.payload_end; ++pos)
	{
		uint32_t offset = pos - pkt.payload_start;

		if (0u == (offset % 32u))
		{
			std::cout << "  +0x" << std::hex << std::setw(6u) << std::setfill('0') << offset;
		}


		std::cout << ' ' << std::hex << std::setw(2u) << std::setfill('0') << static_cast<uint32_t>(*pos);

		if (0u == ((offset + 1u) % 32u))
		{
			std::cout << std::endl;
		}
	}

	if (00 != (pkt.payload_end - pkt.payload_start) % 16u)
	{
		std::cout << std::endl;
	}

	std::cout << std::endl;
	return true;
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
				<< "Dumps command packets of a Xilinx 7-series bitstream." << std::endl
				<< std::endl << std::endl;
			return EXIT_FAILURE;
		}

		bitstream::parse(argv[1], dump_packet);

		return EXIT_SUCCESS;
	}
	catch (std::exception& e)
	{
		std::cerr << std::endl << "error: unhandled exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
