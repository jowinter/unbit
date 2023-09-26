/**
 * @file
 * @brief Common infrastructure for Xilinx 7-Series FPGA block RAM tiles.
 */
#include "fpga/xilinx/bram.hpp"
#include "fpga/xilinx/bitstream.hpp"

#include <cassert>
#include <ostream>

namespace fpga
{
	namespace xilinx
	{
		namespace v7
		{
			//--------------------------------------------------------------------------------------------------------------------
			bram::bram(unsigned x, unsigned y, size_t num_words, size_t data_bits, size_t parity_bits, bram_category category,
				   size_t bitstream_offset, unsigned slr)
				: slr_(slr), x_(x), y_(y), num_words_(num_words), data_bits_(data_bits), parity_bits_(parity_bits), category_(category),
				  bitstream_offset_(bitstream_offset)
			{
			}

			//--------------------------------------------------------------------------------------------------------------------
			bram::~bram()
			{
			}

			//--------------------------------------------------------------------------------------------------------------------
			size_t bram::map_via_table(size_t bit_addr, const uint32_t* map_table, size_t table_size) const
			{
				assert(map_table != nullptr);
				assert(table_size > 0u);

				if (bit_addr >= table_size)
					throw std::out_of_range("bit address to be mapped is out of bounds");

				return bitstream_offset_ + map_table[bit_addr];
			}

			//--------------------------------------------------------------------------------------------------------------------
			std::vector<uint8_t> bram::extract(const bitstream& bits, bool extract_parity) const
			{
				// Determine the length (in bits)
				const size_t bit_length = (extract_parity ? parity_bits_ : data_bits_) * num_words_;
				const size_t byte_length = (bit_length + 7u) / 8u;

				// Prepare the result array
				std::vector<uint8_t> extracted(byte_length);

				// Extract data (bit-wise)
				for (size_t i = 0u; i < bit_length; ++i)
				{
					const size_t src_bit = map_to_bitstream(i, extract_parity);

					// Extract the source value and update the extracted byte array
					if (bits.read_frame_data_bit(src_bit, slr_))
						extracted[i / 8u] |= 1u << (i % 8u);
				}

				// Done
				return extracted;
			}

			//--------------------------------------------------------------------------------------------------------------------
			void bram::inject(bitstream& bits, bool inject_parity, const std::vector<uint8_t>& data) const
			{
				// Determine the length (in bits)
				const size_t bit_length = (inject_parity ? parity_bits_ : data_bits_) * num_words_;
				const size_t byte_length = (bit_length + 7u) / 8u;

				if (data.size() != byte_length)
					throw std::invalid_argument("size of data to be injected does not match block ram size");

				// Extract data (bit-wise)
				for (size_t i = 0u; i < bit_length; ++i)
				{
					const size_t dst_bit = map_to_bitstream(i, inject_parity);

					const size_t src_byte_index = (i / 8u);
					const size_t src_bit_shift = (i % 8u);

					const bool src_value = static_cast<bool>((data[src_byte_index] >> src_bit_shift) & 1u);

					// Inject into the bitstream
					bits.write_frame_data_bit(dst_bit, src_value, slr_);
				}
			}

			//--------------------------------------------------------------------------------------------------------------------
			std::ostream& operator<< (std::ostream& stm, const bram& ram)
			{
				return (stm << std::dec << ram.primitive() << "_X" << ram.x() << "Y" << ram.y());
			}
		}
	}
}
