/**
 * @file
 * @brief Proof-of-concept tool to dump a (processor) image from block RAMs in a Xilinx FPGA.
 */

#include "fpga/xilinx/bitstream.hpp"
#include "fpga/xilinx/bram.hpp"
#include "fpga/xilinx/mmi.hpp"
#include "fpga/xilinx/fpga.hpp"

#include "xml/xml.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>

using unbit::xilinx::bitstream;
using unbit::xilinx::bram;
using unbit::xilinx::bram_category;
using unbit::xilinx::fpga;
using unbit::xilinx::fpga_by_idcode;
using unbit::xilinx::mmi::memory_map;
using unbit::xilinx::mmi::memory_region;

using unbit::xml::xml_parser_guard;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Prints a single Intel-Hex character
 */
static uint8_t ihex_print_byte(uint8_t c)
{
	static const char xdigits[] = "0123456789ABCDEF";

	std::cout << xdigits[(c >> 4u) & 0xFu]
			  << xdigits[c & 0xFu];
	return c;
}

/**
 * @brief Prints an Intel-Hex record
 */
static void ihex_print_record(uint8_t type, uint32_t addr, const uint8_t* payload, unsigned payload_len)
{
	uint8_t chksum = 0u;

	// Start of record
	std::cout << ':';

	// Length of payload data (limited to 65535 bytes)
	chksum += ihex_print_byte(payload_len & 0xFFu);

	// Address (lower 16-bit)
	chksum += ihex_print_byte((addr >> 8u) & 0xFFu);
	chksum += ihex_print_byte(addr & 0xFFu);

	// Record type
	chksum += ihex_print_byte(type);

	// Payload data (if any)
	for (unsigned i = 0u; i < payload_len; ++i)
	{
		chksum += ihex_print_byte(payload[i]);
	}

	// Checksum and end
	ihex_print_byte(-chksum & 0xFFu);
	std::cout << std::endl;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Dumps a memory region (address space)
 */
static void dump_region(const bitstream& bs, const fpga& fpga, const memory_map& mmi, size_t index)
{
	const auto& rgn = mmi.region(index);

	const auto start_byte_addr = rgn.start_bit_addr() / 8u;
	const auto end_byte_addr   = rgn.end_bit_addr() / 8u;
	const size_t byte_size = end_byte_addr - start_byte_addr + 1u;

	uint32_t ihex_current_segment = ~(start_byte_addr & 0xFFFF0000u);
	uint8_t dump_data[16u] = { };

	for (size_t line_offset = 0u; line_offset < byte_size; line_offset += sizeof(dump_data))
	{
		// Track linear address records
		uint32_t ihex_next_segment = (start_byte_addr + line_offset) & 0xFFFF0000u;
		if (ihex_next_segment != ihex_current_segment)
		{
			// Provide an extended linear address record
			const uint8_t data[] =
			{
				static_cast<uint8_t>((ihex_next_segment >> 24u) & 0xFFu),
				static_cast<uint8_t>((ihex_next_segment >> 16u) & 0xFFu)
			};

			ihex_print_record(0x04u, 0x0000u, data, sizeof(data));
			ihex_current_segment = ihex_next_segment;
		}

		// Dump up to 16 hex bytes
		const size_t record_len = std::min(sizeof(dump_data), byte_size - line_offset);
		for (size_t i = 0u; i < record_len; ++i)
		{
			dump_data[i] = mmi.read_byte(fpga, bs, start_byte_addr + line_offset + i);
		}

		// Data record (type 0x00)
		ihex_print_record(0x00u, (line_offset & 0x0000FFFFu), dump_data, record_len);
	}
}

//---------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	xml_parser_guard parser_guard;

	try
	{
		if (argc != 4)
		{
			std::cerr << "usage: " << argv[0u] << " <bitstream> <mmi> <instance>" << std::endl
					  << std::endl;
			return EXIT_FAILURE;
		}

		const bitstream bs = bitstream::load_bitstream(argv[1u], 0xFFFFFFFFu, true);
		const fpga& fpga = fpga_by_idcode(bs.idcode());
		const auto mmi = memory_map::load(argv[2u], argv[3u]);

		// Dump all defined regions (as Intel-Hex dump; our file has sorted them by increasing
		// start byte address)
		for (size_t i = 0u; i < mmi->num_regions(); ++i)
		{
			dump_region(bs, fpga, *mmi, i);
		}

		// End of file record
		ihex_print_record(0x01u, 0x0000u, nullptr, 0u);

		return EXIT_SUCCESS;
	}
	catch (std::exception& e)
	{
		std::cerr << std::endl << "error: unhandled exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
