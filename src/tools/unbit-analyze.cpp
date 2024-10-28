/**
 * @file
 * @brief Bitstream analysis tool for Xilinx 7-Series and Virtuex UltraScale FPGAs.
 */
 #include "unbit/fpga/xilinx/bitstream_engine.hpp"
 #include "unbit/fpga/xilinx/config_engine.hpp"
 #include "unbit/fpga/xilinx/config_context.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <vector>

using unbit::fpga::xilinx::config_engine;
using unbit::fpga::xilinx::config_reg;
using unbit::fpga::xilinx::bitstream_engine;

//------------------------------------------------------------------------------------------
std::vector<uint32_t> load_binary_data(std::istream& f, bool reverse = true)
{
	// Step 0: Skip over leading garbage data until we see the first sync word.
	uint32_t sync_w = 0;

	while (sync_w != bitstream_engine::FPGA_SYNC_WORD_LE)
	{
		std::istream::int_type c = f.get();

		if (f.fail() || (c == std::istream::traits_type::eof()))
			throw std::ios_base::failure("i/o error while scanning for sync word in raw bitstream.");

		sync_w <<= 8u;
		sync_w |= (c & 0xFFu);
	}

	// Rewind to the sync word itself
	f.seekg(-4, std::ios_base::cur);

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
	class unbit_analyzer final : public config_engine
	{
	private:
		uint32_t slr_cfg_index;
		std::size_t slr_offset;

	public:
		unbit_analyzer();
		~unbit_analyzer();

		bool on_config_nop(config_reg reg, word_span_type data) override;
		bool on_config_write(config_reg reg, word_span_type data) override;
		bool on_config_read(config_reg reg, word_span_type data) override;

		void on_config_slr(word_span_type data, uint32_t next_slr_index) override;
	};

	//-----------------------------------------------------------------------------------------------------------------
	unbit_analyzer::unbit_analyzer()
		: slr_cfg_index(0), slr_offset(0)
	{
	}

	//-----------------------------------------------------------------------------------------------------------------
	unbit_analyzer::~unbit_analyzer()
	{
	}

	//-----------------------------------------------------------------------------------------------------------------
	bool unbit_analyzer::on_config_write(config_reg reg, word_span_type data)
	{
		auto& ctx = get_context();

		std::cout << "SLR(" << ctx.slr_index() << ") WRITE REG(" << reg << ")" << " LEN=" << data.size() << std::endl;

		return config_engine::on_config_write(reg, data);
	}

	//-----------------------------------------------------------------------------------------------------------------
	bool unbit_analyzer::on_config_read(config_reg reg, word_span_type data)
	{
		auto& ctx = get_context();

		std::cout << "SLR(" << ctx.slr_index() << ") READ REG(" << reg << ")" << " LEN=" << data.size() << std::endl;

		return config_engine::on_config_write(reg, data);
	}

	//-----------------------------------------------------------------------------------------------------------------
	bool unbit_analyzer::on_config_nop(config_reg reg, word_span_type data)
	{
		// std::cout << "NOP" << std::endl;
		return config_engine::on_config_nop(reg, data);
	}

	//-----------------------------------------------------------------------------------------------------------------
	void unbit_analyzer::on_config_slr(word_span_type data, uint32_t next_slr_index)
	{
		auto& ctx = get_context();

		std::cout << "--- ENTER SLR(" << ctx.slr_index() << ") ---" << std::endl;
		config_engine::on_config_slr(data, next_slr_index);

		std::cout << "IDCODE: " << ctx.idcode().value_or(0u) << std::endl
			<< "FAR: " << ctx.far() << std::endl;
		std::cout << "--- LEAVE SLR(" << ctx.slr_index() << ") ---" << std::endl;
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
		const auto input = load_binary_data(stm);

		auto [n_parsed, success] = unbit_analyzer().process(input.begin(), input.end());
		if (!success)
		{
			std::clog << "ERR: parsing stopped early at word offset 0x" << std::hex << n_parsed << " of 0x" << input.size() << std::dec << std::endl;
		}
		else
		{
			std::clog << "INFO: successfully parsed " << n_parsed << " words (of " << input.size() << " total)" << std::endl;
		}

		return EXIT_SUCCESS;
	}
	catch (std::exception& e)
	{
		std::cerr << std::endl << "error: unhandled exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
