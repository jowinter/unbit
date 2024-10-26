/**
 * @file
 * @brief Detail implementation of support for Xilinx Memory Map Information (MMI) files.
 */
#ifndef UNBIT_XILINX_MMI_DETAIL_HPP_
#define UNBIT_XILINX_MMI_DETAIL_HPP_ 1

#include "unbit/fpga/old/xilinx/mmi.hpp"
#include "unbit/xml/xml.hpp"

namespace unbit
{
	namespace old
	{
		namespace xilinx
		{
			namespace mmi
			{
				using xml::xml_doc;
				using xml::xml_node;

				/**
				* @brief Memory map based on a processor block.
				*/
				class cpu_memory_map : public memory_map
				{
				public:
					/**
					* @brief Parse information about a (placed) block ram.
					*/
					struct mmi_bram
					{
						/** @brief Type of block RAM */
						bram_category type;
						/** @brief X coordinate of the block RAM */
						unsigned x;
						/** @brief Y coordinate of the block RAM */
						unsigned y;
					};

					/**
					* @brief Parse information about a bitlane
					*/
					struct mmi_bitlane
					{
						/** @brief BRAM location of this lane */
						mmi_bram bram;

						/** @brief Start (word) address of this lane */
						unsigned start_word_addr;

						/** @brief End (word) address of this lane */
						unsigned end_word_addr;

						/** @brief Normalized MSB bit location of this lane */
						unsigned msb;

						/** @brief Normalized LSB bit location of this lane */
						unsigned lsb;

						/** @brief Number of parity bits used in this lane */
						unsigned parity_bits;

						/** @brief Bit-reversal indicator (input msb < input lsb) */
						bool bitrev;
					};

					/**
					* @brief Parse information about an address space.
					*/
					struct mmi_space : public memory_region
					{
					public:
						/** @brief Constructs an address space */
						mmi_space();

						/** @brief Destroys an address space */
						virtual ~mmi_space();

					public:
						/** @brief Name of this address space */
						std::string region_name;

						/** @brief Bitlanes of this address space */
						std::vector<mmi_bitlane> lanes;

						/** @brief Start (byte) address of this address space */
						uint64_t start_byte_addr;

						/** @brief End (byte) address of this address space */
						uint64_t end_byte_addr;

						/** @brief Total size (in word) of this address space */
						size_t total_num_words;

						/** @brief Word size (in bits) of this address space */
						size_t word_size;

					public:
						// region interface
						virtual const std::string& name() const override;
						virtual uint64_t start_bit_addr() const override;
						virtual uint64_t end_bit_addr() const override;
					};

				private:
					/**
					* @brief Memory layout (all address spaces)
					*/
					const std::vector<mmi_space> spaces_;

					/**
					* @brief Name of this processor instance
					*/
					const std::string name_;

					/**
					* @brief Endianness defined for this processor
					*/
					const endian endianness_;

				public:
					/**
					* @brief Constructs a memory map from the given processor node.
					*
					* @param[in] xdoc is the XML document containing the processor node.
					*
					* @param[in] xproc is the processor node in an MMI file defining
					*   this CPU's address spaces.
					*/
					cpu_memory_map(xml_doc& xdoc, xml_node& xproc);

					/**
					* @brief Disposes this memory map.
					*/
					virtual ~cpu_memory_map();

					/**
					* @brief Gets the byte endianness of the memory map.
					*/
					virtual endian endianness() const override;

					/**
					* @brief Gets the number of region (address space) in this memory map.
					*/
					virtual size_t num_regions() const override;

					/**
					* @brief  Gets a reference to an region (address space) in this memory map.
					*
					* @param[in] index is the zero based index of the region.
					*/
					virtual const memory_region& region(size_t index) const override;

					/**
					* @brief Reads a single bit
					*/
					virtual bool read_bit(const fpga& fpga, const bitstream& bs,
										uint64_t bit_addr) const override;

					/**
					* @brief Writes a single bit
					*/
					virtual void write_bit(bitstream& bs, const fpga& fpga,
										uint64_t bit_addr, bool value) const override;
				protected:
					/**
					* @brief Maps a bit address to an address.
					*/
					const mmi_space& map_to_space(uint64_t bit_addr) const;

					/**
					* @brief Maps a bit address to a bit lane in a given address space.
					*/
					const mmi_bitlane& map_to_lane(const mmi_space &space,
												uint64_t bit_addr) const;

					/**
					* @brief Maps a bit address to the underlying block RAMs
					*/
					std::tuple<mmi_bram, unsigned, bool>
					map_bit_address(uint64_t bit_addr) const;
				};
			}
		}
	}
}

#endif // #ifndef UNBIT_XILINX_MMI_DETAIL_HPP_
