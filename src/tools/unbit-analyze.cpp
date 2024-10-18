/**
 * @file
 * @brief Bitstream analysis tool for Xilinx 7-Series and Virtuex UltraScale FPGAs.
 */
 #include "fpga/xilinx/bitstream_engine.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <vector>

using unbit::xilinx::bitstream_engine;

//------------------------------------------------------------------------------------------
std::vector<uint32_t> load_binary_data(std::istream& f, bool reverse = true)
{
	// Step 1: Determine the remaining length in the input stream
	size_t size;
	{
		auto start = f.tellg();
		f.seekg(0, std::ios_base::end);

		auto end = f.tellg();

		// Rewind to the initial file position (this allows the user to manually skip extra
		// bytes at the start of f).
		f.seekg(start, std::ios_base::beg);

		if (f.fail())
			throw std::ios_base::failure("i/o error while determining size of bitstream.");

		size = static_cast<size_t>(end - start) / sizeof(uint32_t);
	}

	// Step 2: Read the inomcing data into memory
	std::vector<uint32_t> raw_data(size);
	{
		static_assert(sizeof(std::istream::char_type) == sizeof(uint8_t),
					"unsupported: sizeof(std::istream::char_type) != sizeof(uint8_t)");

		f.read(reinterpret_cast<std::istream::char_type*>(raw_data.data()), size * sizeof(uint32_t));

		if (f.fail())
			throw std::ios_base::failure("i/o error while reading bitstream data.");
	}

	// Step 3: Reverse endianness (if needed)
	for (auto& w : raw_data)
	{
		uint32_t v = ((w >> 24u) & 0xFF)
			| (((w >> 16u) & 0xFF) << 8u)
			| (((w >>  8u) & 0xFF) << 16u)
			| (((w >>  0u) & 0xFF) << 24u);

		w = v;
	}

	return raw_data;
}

//---------------------------------------------------------------------------------------------------------------------
namespace
{
	class unbit_analyzer final : public bitstream_engine
	{
	public:
		unbit_analyzer();
		~unbit_analyzer();

		virtual bool on_config_write(uint32_t reg, const uint32_t *data, std::size_t len) override;
		virtual bool on_config_read(uint32_t reg, const uint32_t *data, std::size_t len) override;
		virtual bool on_config_rsvd(uint32_t reg, const uint32_t *data, std::size_t len) override;
		virtual bool on_config_nop(uint32_t reg, const uint32_t *data, std::size_t len) override;
	};

	//-----------------------------------------------------------------------------------------------------------------
	unbit_analyzer::unbit_analyzer()
	{
	}

	//-----------------------------------------------------------------------------------------------------------------
	unbit_analyzer::~unbit_analyzer()
	{
	}

	//-----------------------------------------------------------------------------------------------------------------
	bool unbit_analyzer::on_config_write(uint32_t reg, const uint32_t *data, std::size_t len)
	{
		std::cout << "WRITE REG(" << reg << ")" << " LEN=" << len << std::endl;
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------
	bool unbit_analyzer::on_config_read(uint32_t reg, const uint32_t *data, std::size_t len)
	{
		std::cout << "READ REG(" << reg << ")" << std::endl;
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------
	bool unbit_analyzer::on_config_rsvd(uint32_t reg, const uint32_t *data, std::size_t len)
	{
		std::cout << "RSVD REG(" << reg << ")" << std::endl;
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------
	bool unbit_analyzer::on_config_nop(uint32_t reg, const uint32_t *data, std::size_t len)
	{
		// std::cout << "NOP" << std::endl;
		return true;
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
				<< "Analyzes a Xilinx 7-series or Virtex UltraScale+ bitstream." << std::endl
				<< std::endl << std::endl;
			return EXIT_FAILURE;
		}

		std::ifstream stm(argv[1], std::ios_base::in | std::ios_base::binary);
		auto input = load_binary_data(stm);

		unbit_analyzer analyzer;
		
		std::size_t n_parsed = analyzer.process(input.data(), input.size());

		if (n_parsed != input.size())
		{
			std::clog << "ERR: parsing stopped early at word offset 0x" << std::hex << n_parsed << " of 0x" << input.size() << std::dec << std::endl;
		}

		return EXIT_SUCCESS;
	}
	catch (std::exception& e)
	{
		std::cerr << std::endl << "error: unhandled exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
