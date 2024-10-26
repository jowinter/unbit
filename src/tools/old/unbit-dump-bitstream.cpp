/**
 * @file
 * @brief Proof-of-concept tool to dump the structure of a Xilinx 7-series bitstream
 */

#include "unbit/fpga/old/xilinx/bitstream.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>

using unbit::old::xilinx::bitstream;

//---------------------------------------------------------------------------------------------------------------------
// Well-known registers
//
static const char* type1_reg_name(uint32_t reg)
{
	// cf. Xilinx UG470, Table 5-23 "Type 1 Packet Registers"
	switch (reg)
	{
	case 0b00000: return "CRC";
	case 0b00001: return "FAR";
	case 0b00010: return "FDRI";
	case 0b00011: return "FDRO";
	case 0b00100: return "CMD";
	case 0b00101: return "CTL0";
	case 0b00110: return "MASK";
	case 0b00111: return "STAT";
	case 0b01000: return "LOUT";
	case 0b01001: return "COR0";
	case 0b01010: return "MFWR";
	case 0b01011: return "CBC";
	case 0b01100: return "IDCODE";
	case 0b01101: return "ACSS";
	case 0b01110: return "COR1";
	case 0b10000: return "WBSTAR";
	case 0b10001: return "TIMER";
	case 0b10011: return "RBCRC_SW";
	case 0b10110: return "BOOTSTS";
	case 0b11000: return "CTL1";
	case 0b11111: return "BSPI";

		// Not documented in UG470. Seen at start of a new "(sub-)bitstream", e.g. when
		// switching between SLRs. Followed by a type2 packet with the data for the substream
	case 0b11110: return "slave?";

	default:      return "reg?";
	}
}

//---------------------------------------------------------------------------------------------------------------------
// Well-known type1 operations
//
static const char* type1_op_name(uint32_t op)
{
	switch (op)
	{
	case 0b00: return "read";
	case 0b01: return "op1";
	case 0b10: return "write";
	case 0b11: return "op3";
	default:   return "op?";
	}
}

//---------------------------------------------------------------------------------------------------------------------
// Well-known commands (for type1 cmd regiter
//
static const char* type1_cmd_name(const bitstream::packet& pkt)
{
	uint32_t cmd = 0xFFFFFFFF;

	if (pkt.word_count == 1u)
	{
		// Exactly one word of payload is present, decode it.
		auto cfg_pos = pkt.payload_start;

		cmd =  static_cast<uint32_t>(*cfg_pos++) << 24u;
		cmd |= static_cast<uint32_t>(*cfg_pos++) << 16u;
		cmd |= static_cast<uint32_t>(*cfg_pos++) <<  8u;
		cmd |= static_cast<uint32_t>(*cfg_pos++);
	}

	switch (cmd)
	{
	case 0b00000: return "NULL";
	case 0b00001: return "WCFG";
	case 0b00010: return "MFW";
	case 0b00011: return "LFRM";
	case 0b00100: return "RCFG";
	case 0b00101: return "START";
	case 0b00110: return "RCAP";
	case 0b00111: return "RCRC";
	case 0b01000: return "AGHIGH";
	case 0b01001: return "SWITCH";
	case 0b01010: return "GRESTORE";
	case 0b01011: return "SHUTDOWN";
	case 0b01100: return "GCAPTURE";
	case 0b01101: return "DESYNC";
	case 0b01110: return "RESERVED";
	case 0b01111: return "IPROG";
	case 0b10000: return "CRCC";
	case 0b10001: return "LTIMER";
	case 0b10010: return "BSPI_READ";
	case 0b10011: return "FALL_EDGE";
	default: return "cmd?";
	}
}

//---------------------------------------------------------------------------------------------------------------------
static bool dump_packet(const bitstream::packet& pkt)
{
	std::cout << "[" << std::hex
		  << std::setw(8u) << std::setfill('0') << pkt.storage_offset
		  << " " << std::setw(2u) << std::setfill('0') << pkt.stream_index
		  << ":" << std::setw(8u) << std::setfill('0') << pkt.offset
		  << "] 0x" << std::setw(8u) << std::setfill('0') << pkt.hdr;

	// Breakdown for packet dump
	bool should_dump = true;

	if (pkt.packet_type == 0x1)
	{
		// Type 1 packet
		if (pkt.hdr == 0x20000000u)
		{
			// Type 1 read from CRC register is used as NOP
			std::cout << " type1 nop";
		}
		else
		{
			std::cout << " type1 " << type1_op_name(pkt.op)
				  << " reg=0x" << std::setw(2) << std::setfill('0') << pkt.reg
				  << " [" << type1_reg_name(pkt.reg) << "]";

			// For command packets: show the command name
			if (pkt.reg == 0b00100)
			{
				std::cout << " " << type1_cmd_name(pkt);
			}
		}
	}
	else if (pkt.packet_type == 0x2)
	{
		// Type 2 packet
		std::cout << " type2 op=0x" << std::setw(1) << std::setfill('0') << pkt.op;
	}

	if (pkt.word_count > 0u)
	{
		std::cout << " nwords=" <<  std::dec << pkt.word_count;
	}

	// Omit the hexdump of "slave?" packets (as we will parse into it).
	if (pkt.op == 0b10 && pkt.reg == 0b11110 && pkt.word_count > 0u)
	{
		std::cout << std::endl << "  ---8x---8x--- switch to new (sub-)stream ---8x---8x---";
		should_dump = false;
	}

	std::cout << std::endl;

	// Append a hexdump of the payload
	for (auto pos = pkt.payload_start; should_dump && pos != pkt.payload_end; ++pos)
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

	if (0 != (pkt.payload_end - pkt.payload_start) % 16u)
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
				<< "Dumps command packets of a Xilinx 7-series or Virtex UltraScale+ bitstream." << std::endl
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
