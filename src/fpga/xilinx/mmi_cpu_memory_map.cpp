/**
 * @file
 * @brief Detail implementation of support for Xilinx Memory Map Information (MMI) files.
 */
#include "mmi_detail.hpp"

#include <algorithm>
#include <cstdio>
#include <tuple>

using unbit::xml::xml_doc;
using unbit::xml::xml_node;
using unbit::xml::xpath_context;

namespace unbit
{
	namespace xilinx
	{
		namespace mmi
		{
			namespace
			{
				//-------------------------------------------------------------------------------------
				/**
				 * @brief Cast a 64-bit value to a 32-bit range (with out of bounds check)
				 */
				static uint32_t safe_to_u32(uint64_t value, uint32_t max_value = UINT32_MAX,
											uint32_t min_value = 0u)
				{
					if (value < min_value || value > max_value)
					{
						throw std::runtime_error("out of range value conversion");
					}

					return static_cast<uint32_t>(value);
				}

				//-------------------------------------------------------------------------------------
				/**
				 * @brief Extracts the word-endianness indication from a processor specification.
				 */
				static auto get_processor_endianness(xml_node& xproc)
				{
					auto endianness = xproc.attribute("Endianness");
					if (endianness == "Little")
					{
						// Little-endian CPU
						return endian::little;
					}
					else if (endianness == "Big")
					{
						// Big-endian CPU
						return endian::big;
					}
					else
					{
						// Not recognized
						throw std::runtime_error("unrecognized processor endianness");
					}
				}

				//-------------------------------------------------------------------------------------
				/**
				 * @brief Extracts the block-ram type and placement from a bitlane specification.
				 */
				static auto get_bitlane_bram(xml_node& xlane)
				{
					auto type = xlane.attribute("MemType");
					auto placement = xlane.attribute("Placement");

					cpu_memory_map::mmi_bram result { };

					// Step 1: Decode the block RAM type
					if (type == "RAMB36")
					{
						result.type = bram_category::ramb36;
					}
					else if (type == "RAMB18")
					{
						result.type = bram_category::ramb18;
					}
					else
					{
						// Not recognized
						throw std::runtime_error("unrecognized block ram type");
					}

					// Step 2: Parse the placement
					int r = std::sscanf(placement, "X%uY%u", &result.x, &result.y);
					if (r != 2)
					{
						// Not recognized
						throw std::runtime_error("unrecognized block ram placement");
					}

					return result;
				}

				//-------------------------------------------------------------------------------------
				/**
				 * @brief Extracts information about a single bitlane
				 */
				static auto add_mmi_bitlane(std::vector<cpu_memory_map::mmi_bitlane>& lanes,
											 xpath_context& xpath, xml_node& xlane)
				{
					cpu_memory_map::mmi_bitlane& result = lanes.emplace_back();

					// RAM type and placement
					result.bram = get_bitlane_bram(xlane);

					// Bit slice of the lane (we currently support up to 64 bits per lane
					auto xdatawidth = xpath.query(xlane, "./DataWidth").node_at(0u);
					result.msb = safe_to_u32(xdatawidth.attribute_as_uint64("MSB"));
					result.lsb = safe_to_u32(xdatawidth.attribute_as_uint64("LSB"));

					// Check fro bit-reversal at lane level (LSB > MSB)
					result.bitrev = (result.msb < result.lsb);
					if (result.bitrev)
					{
						// This is a bit-reversed lane, normalize msb/lsb order
						std::swap(result.msb, result.lsb);
					}

					// Word address range of the lane (relative to the address space)
					auto xrange = xpath.query(xlane, "./AddressRange").node_at(0u);
					result.start_word_addr = safe_to_u32(xrange.attribute_as_uint64("Begin"));
					result.end_word_addr   = safe_to_u32(xrange.attribute_as_uint64("End"));

					if (result.end_word_addr < result.start_word_addr)
					{
						throw std::runtime_error("malformed input file (end address of bitlane below start address");
					}

					// Use of parity bits
					auto xparity = xpath.query(xlane, "./Parity").node_at(0u);
					result.parity_bits = (xparity.attribute("ON") == "true") ?
						safe_to_u32(xparity.attribute_as_uint64("NumBits")) : 0u;

					// And append
					return result;
				}

				//-------------------------------------------------------------------------------------
				/**
				 * @brief Extracts information about an address space and its bit lanes
				 */
				static auto add_mmi_space(std::vector<cpu_memory_map::mmi_space>& spaces,
										  xpath_context& xpath, xml_node& xspace)
				{
					cpu_memory_map::mmi_space& result = spaces.emplace_back();

					// Extract common metadata
					result.region_name     = xspace.attribute("Name");
					result.start_byte_addr = xspace.attribute_as_uint64("Begin");
					result.end_byte_addr   = xspace.attribute_as_uint64("End");

					// Total word size is given by the slices in the bit lanes
					unsigned word_msb = 0u;
					unsigned word_lsb = UINT32_MAX;

					// Extract the bit lanes
					auto xlanes = xpath.query(xspace, "./BusBlock/BitLane");
					result.lanes.reserve(xlanes.node_count());

					for (auto i = 0u; i < xlanes.node_count(); ++i)
					{
						auto xlane = xlanes.node_at(i);

						auto lane = add_mmi_bitlane(result.lanes, xpath, xlane);
						word_msb = std::max(word_msb, lane.msb);
						word_lsb = std::min(word_lsb, lane.lsb);
					}

					if (word_msb < word_lsb)
					{
						throw std::runtime_error("infeasible address space (normalized msb < "
												 "normalized lsb; no bitlanes defined?)");
					}

					// Infer the word size and total (bit) size
					result.word_size = word_msb - word_lsb + 1u;
					if (result.word_size % 8u != 0u)
					{
						// Implementation limit (currently)
						throw std::runtime_error("unsupported address space (word size is not a multiple of 8 bits)");
					}

					size_t total_bit_size = (result.end_byte_addr - result.start_byte_addr + 1u) * 8u;
					if (total_bit_size % result.word_size != 0u)
					{
						throw std::runtime_error("infeasible address space (total bit size is "
												 "not an integer multiple of the word size)");
					}

					result.total_num_words = total_bit_size / result.word_size;

					return result;
				}
			}

			//-------------------------------------------------------------------------------------
			/**
			 * @brief Extracts information all address spaces of a given processor.
			 */
			static auto get_mmi_spaces(xml_doc& xdoc, xml_node& xproc)
			{
				std::vector<cpu_memory_map::mmi_space> result;

				xpath_context xpath(xdoc);

				// Scan over the address spaces
				auto xspaces = xpath.query(xproc, "./AddressSpace");
				result.reserve(xspaces.node_count());

				for (auto i_space = 0u; i_space < xspaces.node_count(); ++i_space)
				{
					// Extract all address spaces
					auto xspace = xspaces.node_at(i_space);
					add_mmi_space(result, xpath, xspace);
				}

				// Sort regions by increasing order of start byte address
				std::sort(result.begin(), result.end(),
				  [] (const auto& a, const auto& b)
				  {
					  return a.start_byte_addr < b.start_byte_addr;
				  });

				return result;
			}

			//-------------------------------------------------------------------------------------
			cpu_memory_map::cpu_memory_map(xml_doc& xdoc, xml_node& xproc)
				: spaces_(get_mmi_spaces(xdoc, xproc)),
				  name_(xproc.attribute("InstPath")),
				  endianness_(get_processor_endianness(xproc))
			{
			}

			//-------------------------------------------------------------------------------------
			cpu_memory_map::~cpu_memory_map()
			{
			}

			//-------------------------------------------------------------------------------------
			endian cpu_memory_map::endianness() const
			{
				return endianness_;
			}

			//-------------------------------------------------------------------------------------
			size_t cpu_memory_map::num_regions() const
			{
				return spaces_.size();
			}

			//-------------------------------------------------------------------------------------
			const memory_region& cpu_memory_map::region(size_t index) const
			{
				return spaces_.at(index);
			}

			//-------------------------------------------------------------------------------------
			const cpu_memory_map::mmi_space&
			cpu_memory_map::map_to_space(uint64_t bit_addr) const
			{
				const auto byte_addr = bit_addr / 8u;

				for (const auto& space : spaces_)
				{
					if (space.start_byte_addr <= byte_addr && byte_addr <= space.end_byte_addr)
					{
						return space;
					}
				}

				// Mapping failed
				throw std::invalid_argument("failed to map bit to address space");
			}

			//-------------------------------------------------------------------------------------
			const cpu_memory_map::mmi_bitlane&
			cpu_memory_map::map_to_lane(const cpu_memory_map::mmi_space &space,
										uint64_t bit_addr) const
			{
				const auto offset = bit_addr % space.word_size;

				for (const auto& lane : space.lanes)
				{
					if (lane.lsb <= offset && offset <= lane.msb)
					{
						return lane;
					}
				}


				// Mapping failed
				throw std::invalid_argument("failed to map bit to lane");
			}

			//-------------------------------------------------------------------------------------
			std::tuple<cpu_memory_map::mmi_bram, unsigned, bool>
			cpu_memory_map::map_bit_address(uint64_t bit_addr) const
			{
				// Step 1: Find the containing address space
				const auto& space = map_to_space(bit_addr);

				// Step 2: Within an address space, map to a bit lane
				const auto& lane = map_to_lane(space, bit_addr);

				// Step 3: Find the offset within the target BRAM
				//
				// - Each bitlane defines the mapping of a bit slice to one BRAM
				const uint64_t space_bit_offset  = bit_addr - space.start_byte_addr * 8u;
				const uint64_t space_word_offset = space_bit_offset / space.word_size;

				// FIXME: We do not (yet) handle parity bits correctly ...
				// - For now we throw (to avoid producing bad output)
				//
				// - To handle parity we need to understand the data and parity widths of the
				//   underlying RAM primitive accordingly.
				//
				// - The mapping formula below likely needs an adjustment for the lane_word_size
				//   and the "- lane.lsb" term to accomodate for parity.
				//
				//   (Yet) untested idea:
				//   if lane word bit offset >= first parity bit:
				//      - use parity_bits as lane_word_size for the ram access
				//      - adjust bram_bit_offset by -"first parity bit" inst of "-lane.lsb"
				//
				//
				if (lane.parity_bits > 0)
				{
					throw std::logic_error("parity bits are not (yet) implemented correctly");
				}

				// Map for the data area of the RAM
				const unsigned lane_word_size  = lane.msb - lane.lsb + 1u;
				const unsigned bram_bit_offset = space_word_offset * lane_word_size +
					(space_bit_offset % space.word_size) - lane.lsb;

				return std::make_tuple(lane.bram, bram_bit_offset, false);
			}

			//-------------------------------------------------------------------------------------
			bool cpu_memory_map::read_bit(const fpga& fpga, const bitstream& bs,
										  uint64_t bit_addr) const
			{
				// Map the bit to a block RAM, then extract
				const auto mapping = map_bit_address(bit_addr);

				// Resolve the block RAM
				const auto& bram = fpga.bram_by_loc(std::get<0>(mapping).type,
													std::get<0>(mapping).x,
													std::get<0>(mapping).y);

				// And extract
				return bram.extract_bit(bs, std::get<1>(mapping), std::get<2>(mapping));
			}

			//-------------------------------------------------------------------------------------
			void cpu_memory_map::write_bit(bitstream& bs, const fpga& fpga,
										   uint64_t bit_addr, bool value) const
			{
				// Map the bit to a block RAM, then extract
				const auto mapping = map_bit_address(bit_addr);

				// Resolve the block RAM
				const auto& bram = fpga.bram_by_loc(std::get<0>(mapping).type,
													std::get<0>(mapping).x,
													std::get<0>(mapping).y);

				// And inject
				bram.inject_bit(bs, std::get<1>(mapping), std::get<2>(mapping), value);
			}
		}
	}
}
