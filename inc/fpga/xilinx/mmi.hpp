/**
 * @file
 * @brief Support for Xilinx Memory Map Information (MMI) files.
 */
#ifndef UNBIT_XILINX_MMI_HPP_
#define UNBIT_XILINX_MMI_HPP_ 1

#include "fpga.hpp"
#include "bram.hpp"
#include "bitstream.hpp"

#include <memory>
#include <string>

namespace unbit
{
	namespace xilinx
	{
		namespace mmi
		{
			/**
			 * @brief Endianness of a processor (in an MMI file)
			 *
			 * @note This type can be typedef'd to @c std::endian on newer C++ compilers that provide
			 *   the C++20 @c <bit> header and @c std::endian type.
			 */
			enum class endian
			{
				little,
				big,
				native
			};

			/**
			 * @brief A contigous region (address space) in the memory map.
			 */
			class memory_region
			{
			protected:
				// Not intended for delete through pointer
				virtual ~memory_region() =default;

			public:
				/**
				 * @brief Gets the name of this region.
				 */
				virtual const std::string& name() const = 0;

				/**
				 * @brief Gets the start bit address of this region.
				 */
				virtual uint64_t start_bit_addr() const = 0;

				/**
				 * @brief Gets the end bit address of this region.
				 */
				virtual uint64_t end_bit_addr() const = 0;
			};


			/**
			 * @brief Memory map information.
			 *
			 * The @ref memory_map class describes the mapping between a processor's address space(s), or
			 * a XPM RAM macro, and the underlying BRAM primitives on the FPGA. We represent each memory
			 * map as a set of @ref memory_map::region covering portions of the address space.
			 */
			class memory_map
			{
			protected:
				/**
				 * @brief Constructs a memory map
				 */
				memory_map();

			public:

				/**
				 * @brief Destroys a memory map.
				 */
				virtual ~memory_map();

				/**
				 * @brief Gets the byte endianness of the memory map.
				 *
				 * @note Currently unused by the memory map. Bit-lane extraction is based on
				 *   MSB/LSB slices in the MMI files.
				 */
				virtual endian endianness() const = 0;

				/**
				 * @brief Gets the number of region (address space) in this memory map.
				 */
				virtual size_t num_regions() const = 0;

				/**
				 * @brief  Gets a reference to an region (address space) in this memory map.
				 *
				 * @param[in] index is the zero based index of the region.
				 */
				virtual const memory_region& region(size_t index) const = 0;

				/**
				 * @brief Reads a single bit.
				 *
				 * @param[in] bs is the source bit stream.
				 *
				 * @param[in] bit_addr is the address of the bit in CPU address space.
				 */
				virtual bool read_bit(const fpga& fpga, const bitstream& bs,
									  uint64_t bit_addr) const = 0;

				/**
				 * @brief Reads an 8-bit byte.
				 *
				 * @param[in] fpga is the FPGA type for block RAM translation.
				 *
				 * @param[in] bs is the source bit stream.
				 *
				 * @param[in] byte_addr is the byte address in CPU address space.
				 *
				 * @note The default implementation of this method delegates to @ref read_bit
				 *   for bit-level access.
				 */
				virtual uint8_t read_byte(const fpga& fpga, const bitstream& bs,
										  uint64_t byte_addr) const;

				/**
				 * @brief Writes a single bit.
				 *
				 * @param[in] fpga is the FPGA type for block RAM translation.
				 *
				 * @param[in,out] bs is the source/destination bit stream.
				 *
				 * @param[in] bit_addr is the address of the bit in CPU address space.
				 *
				 * @param[in] value is the bit value to be written.
				 */
				virtual void write_bit(bitstream& bs, const fpga& fpga,
									   uint64_t bit_addr, bool value) const = 0;

				/**
				 * @brief Writes an 8-bit byte.
				 *
				 * @param[in,out] bs is the source/destination bit stream.
				 *
				 * @param[in] fpga is the FPGA type for block RAM translation.
				 *
				 * @param[in] byte_addr is the byte address in CPU address space.
				 *
				 * @param[in] value is the byte value to be written
				 *
				 * @note The default implementation of this method delegates to @ref write_bit
				 *   for bit-level access.
				 */
				virtual void write_byte(bitstream& bs, const fpga& fpga,
										uint64_t byte_addr, uint8_t value) const;

			public:
				/**
				 * @brief Loads a memory map from a given file.
				 *
				 * @param[in] filename specifies the file name (path) of the MMI file to be loaded.
				 *
				 * @param[in] instance is the instance name of the memory to be extracted.
				 *
				 * @return A pointer to the memory map object representing the input file.
				 *
				 * @throws std::runtime_error if parsing of the XML document fails (e.g. file not found)
				 *
				 * @throws std::invalid_argument is the specified instance could not be found.
				 */
				static std::unique_ptr<memory_map> load(const std::string& filename, const std::string& instance);

			protected:
				// Non-copyable
				memory_map(const memory_map& other) =delete;
				memory_map& operator= (const memory_map& other) =delete;
			};
		}
	}
}

#endif // #ifndef UNBIT_XILINX_MMI_HPP_
