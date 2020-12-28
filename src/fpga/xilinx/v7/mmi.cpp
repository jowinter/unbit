/**
 * @file
 * @brief Xilinx BRAM Memory Map Info (MMI) File manipulation
 */
#include "fpga/xilinx/mapper.hpp"
#include "fpga/xilinx/bram.hpp"
#include "fpga/xilinx/zynq7.hpp"
#include "fpga/xilinx/mmi.hpp"

#include "rapidxml/rapidxml.hpp"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cerrno>

namespace fpga
{
	namespace xilinx
	{
		namespace v7
		{
			//-------------------------------------------------------------------------------------------------------------------			
			// XPM parsing context
			//
			struct mmi_parser_imp
			{
			public:
				/** @brief Alias for the XML document type */
				typedef rapidxml::xml_document<std::istream::char_type> xml_document;

				/** @brief Alias for the XML node type */
				typedef rapidxml::xml_node<std::istream::char_type> xml_node;

				/** @brief Alias for the XML data vector type */
				typedef std::vector<std::istream::char_type> xml_data_vector;

				/** @brief Initializer tuple */
				typedef std::tuple<xml_data_vector, xml_document, xml_node, const zynq7&> init_tuple;

			private:
				/**
				 * @brief Backing-store data of the XML document
				 */
				xml_data_vector data;

				/**
				 * @brief The parsed XML document
				 */
				xml_document xml;

				/**
				 * @brief XML root node (cached).
				 */
				xml_node* xroot;

				/**
				 * @brief The FPGA to be used.
				 */
				const zynq7* fpga;

			public:
				mmi_parser_imp(const std::string& filename, mmi_fpga_type_lookup_t fpga_lookup, mmi_instance_filter_t instance_filter);
				~mmi_parser_imp();				

			private:
				static xml_data_vector load_xml_text(const std::string& filename);
				static std::optional<std::string> get_attribute(xml_node* node, const char* name);				
				static std::optional<int64_t> get_attribute_i64(xml_node* node, const char* name);

				static std::string get_required_attribute(xml_node* node, const char* name);
				static int64_t get_required_attribute_i64(xml_node* node, const char* name);

				void parse_rams(mmi_instance_filter_t instance_filter);
				void parse_mem_array(const std::string& inst_path, xml_node* xmemarray);
				void parse_mem_array_dataport(mapper& m, const bram& ram,
					xml_node* xdatawidth, xml_node* xaddrrange, xml_node* xbitlayout,
					int64_t mem_word_size);
				void parse_processor(const std::string& inst_path, xml_node* xprocessor);

				std::vector<std::tuple<unsigned, bool>> parse_bitlayout(const std::string& bitlayout, const bram& ram);
				const bram& resolve_bram(const std::string& type, const std::string& loc);

			private:
				// Non-copyable
				mmi_parser_imp(const mmi_parser_imp&) = delete;
				mmi_parser_imp& operator=(const mmi_parser_imp&) = delete;
			};

			//-------------------------------------------------------------------------------------------------------------------
			mmi_parser_imp::mmi_parser_imp(const std::string& filename, mmi_fpga_type_lookup_t fpga_lookup,
				mmi_instance_filter_t instance_filter)
				: data(load_xml_text(filename)), xroot(nullptr), fpga(nullptr)
			{
				// Parse the XML document
				xml.parse<0>(data.data());

				// Locate the root node
				xroot = xml.first_node("MemInfo");
				if (!xroot)
					throw std::invalid_argument("failed to locate <MemInfo> tag at root of input MMI file.");

				// Detect the FPGA type from the options
				{
					auto xconfig = xroot->first_node("Config");
					if (!xconfig)
						throw std::invalid_argument("failed to locate <Config> tag in input MMI file.");

					for (auto xoption = xconfig->first_node("Option"); xoption; xoption = xoption->next_sibling("Option"))
					{
						auto name = get_attribute(xoption, "Name").value();
						auto value = get_attribute(xoption, "Val").value();

						// Process the options
						if (name == "Part")
						{
							// We are parsing the part-name option
							fpga = &fpga_lookup(value);
						}
						else
						{
							// Unhandled option (ignored)
						}						
					}

					// Sanity check: FPGA given?
					if (!fpga)
						throw std::invalid_argument("failed to determine FPGA type from input MMI file (missing <Option> tag?)");
				}

				// Finally parse the RAMs
				parse_rams(instance_filter);
			}

			//-------------------------------------------------------------------------------------------------------------------
			mmi_parser_imp::~mmi_parser_imp()
			{
			}

			//-------------------------------------------------------------------------------------------------------------------
			mmi_parser_imp::xml_data_vector mmi_parser_imp::load_xml_text(const std::string& filename)
			{
				std::ifstream f(filename, std::ios_base::in | std::ios_base::binary);

				// Step 1: Determine the remaining length in the input stream
				size_t size;
				{
					auto start = f.tellg();
					f.seekg(0, std::ios_base::end);

					auto end = f.tellg();

					// Rewind to the initial file position (this allows to user to manually skip extra bytes at the start of f).
					f.seekg(start, std::ios_base::beg);

					if (f.fail())
						throw std::ios_base::failure("i/o error while determining size of bitstream.");
					
					size = static_cast<size_t>(end - start);
				}

				// Step 2: Read the inomcing data into memory (we allocate a trailing NUL terminator at the end)
				xml_data_vector raw_data(size + 1u, '\0');
				{
					f.read(reinterpret_cast<std::istream::char_type*>(raw_data.data()), size);

					if (f.fail())
						throw std::ios_base::failure("i/o error while reading bitstream data into memory.");
				}

				return raw_data;
			}

			//-------------------------------------------------------------------------------------------------------------------
			std::optional<std::string> mmi_parser_imp::get_attribute(xml_node* node, const char* name)
			{
				if (node)
				{
					// Scan the attributes of this node
					if (auto xattr = node->first_attribute(name))
					{										
						// Return the attribute value
						return std::string(xattr->value());
					}
				}

				// Nothing found (return an empty attribute)
				return {};
			}

			//-------------------------------------------------------------------------------------------------------------------
			std::optional<int64_t> mmi_parser_imp::get_attribute_i64(xml_node* node, const char* name)
			{
				if (auto str_value = get_attribute(node, name))
				{
					// Parse the value
					char* endp = nullptr;
					errno = 0;
					int64_t result = std::strtoll(str_value.value().c_str(), &endp, 0);
					if (errno != 0 || *endp != '\0')
					{
						std::string msg = "malformed input MMI file (failed to parse integer attribute '";
						msg += name;
						msg += "')";
						throw std::invalid_argument(msg);
					}

					return result;
				}

				// No value
				return {};
			}

			//-------------------------------------------------------------------------------------------------------------------
			std::string mmi_parser_imp::get_required_attribute(xml_node* node, const char* name)
			{
				if (auto str_value = get_attribute(node, name))
				{
					return str_value.value();
				}
				else
				{
					std::string msg = "malformed input MMI file (missing string attribute '";
					msg += name;
					msg += "')";
					throw std::invalid_argument(msg);
				}
			}

			//-------------------------------------------------------------------------------------------------------------------
			int64_t mmi_parser_imp::get_required_attribute_i64(xml_node* node, const char* name)
			{
				if (auto i64_value = get_attribute_i64(node, name))
				{
					return i64_value.value();
				}
				else
				{
					std::string msg = "malformed input MMI file (missing integer attribute '";
					msg += name;
					msg += "')";
					throw std::invalid_argument(msg);
				}
			}

			//-------------------------------------------------------------------------------------------------------------------
			void mmi_parser_imp::parse_rams(mmi_instance_filter_t instance_filter)
			{
				const std::string mem_array_tag = "MemoryArray";
				const std::string processor_tag = "Processor";

				// Ensure that there is an instance filter
				if (!instance_filter)
					instance_filter = [] (const std::string&) { return true; };

				// And process
				for (auto xnode = xroot->first_node(); xnode; xnode = xnode->next_sibling())
				{
					// Non-empty instance path, filter accepted
					auto tag_name = xnode->name();

					if (mem_array_tag == tag_name)
					{
						// MemoryArray tag describing an XPM memory macro
						auto inst_path = get_required_attribute(xnode, "InstPath");
						if (instance_filter(inst_path))
						{
							parse_mem_array(inst_path, xnode);
						}
					}
					else if (processor_tag == tag_name)
					{
						// Processor tag describing an embedded processor address space.
						auto inst_path = get_required_attribute(xnode, "InstPath");
						if (instance_filter(inst_path))
						{
							parse_processor(inst_path, xnode);
						}
					}
					else
					{
						// Other memory tag (cuurrenly ignored - TBD: throw?)
					}

				}
			}

			//-------------------------------------------------------------------------------------------------------------------
			void mmi_parser_imp::parse_mem_array_dataport(mapper& m, const bram& ram,
				xml_node* xdatawidth, xml_node* xaddrrange, xml_node* xbitlayout,
				int64_t mem_word_size)			
			{				
				// Extract the MSB/LSB slice from BRAM mapping				
				if (!xdatawidth)
					throw std::invalid_argument("malformed input MMI file (failed to parse <BRAM> without <DataWidth_PortA>)");

				auto msb = get_required_attribute_i64(xdatawidth, "MSB");
				auto lsb = get_required_attribute_i64(xdatawidth, "LSB");
				if (msb < 0 || lsb < 0 || msb >= (std::numeric_limits<int32_t>::max() - 1u) || lsb >= (std::numeric_limits<int32_t>::max() - 1u))
					throw std::invalid_argument("malformed input MMI file (MSB and/or LSB of <DataWidth_PortA> exceed implementation limits)");

				auto slice_width = std::abs(msb - lsb) + 1u;

				if (slice_width > 0u)
				{
					// Extract the address slice (address is given in RAM words)
					if (!xaddrrange)
						throw std::invalid_argument("malformed input MMI file (failed to parse <BRAM> without <AddressRange_PortA>)");

					auto start_addr = get_required_attribute_i64(xaddrrange, "Begin");
					auto end_addr = get_required_attribute_i64(xaddrrange, "End");
					if (start_addr < 0 || end_addr < 0 || start_addr >= std::numeric_limits<int32_t>::max() ||
						end_addr >= std::numeric_limits<int32_t>::max())
						throw std::invalid_argument("malformed input MMI file (MSB and/or LSB of <DataWidth_PortA> exceed implementation limits)");

					if (start_addr > end_addr)
						throw std::invalid_argument("malformed input MMI file (start address of <AddressRange_PortA> must be less or equal than end address)");

					// Parse the bitlayout string
					if (!xbitlayout)
						throw std::invalid_argument("malformed input MMI file (failed to parse <BRAM> without <BitLayout_PortA>)");

					auto bitlayout = parse_bitlayout(get_required_attribute(xbitlayout, "pattern"), ram);

					// And now map the bitlayout string
					// TBD: RAM stride etc.
					//
					// For now we assume:
					//  --> Stride is equal to the mapping slice width (as per bit-layout)
					//  --> Bit-offset in RAM is zero
					//  --> MSB >= LSB (TBD: this should be relaxed in future)
					if (lsb > msb)
						throw std::invalid_argument("TODO: msb < lsb is not yet implemented");

					// Map the slice as given by the bit-layout					
					for (auto map_slice : bitlayout)
					{
						auto map_width = std::get<0>(map_slice);
						auto map_parity = std::get<1>(map_slice);
						auto map_msb = static_cast<unsigned>(msb);
						auto map_lsb = static_cast<unsigned>(msb - map_width) + 1u;

						m.add(start_addr, end_addr, map_lsb, map_msb, ram, 0u, map_width, map_parity);

						// FIXME: Handle reversed byte order ... (do Xilinx tools generate such files?)
						msb -= map_width;
					}
				}
			}

			//-------------------------------------------------------------------------------------------------------------------
			void mmi_parser_imp::parse_mem_array(const std::string& inst_path, xml_node* xmemarray)
			{
				// All specimen seen so far have exactly one MemoryLayout as child of their MemArray
				auto xlayout = xmemarray->first_node("MemoryLayout");
				if (!xlayout)
					throw std::invalid_argument("malformed input MMI file (failed to parse <MemoryArray> without <MemoryLayout>)");

				// Phase 1: Determine the core memory size (from the CoreMemory_Width attribute of the memory layout)
				int64_t mem_word_size = get_required_attribute_i64(xlayout, "CoreMemory_Width");
				if (mem_word_size < 1 || mem_word_size > std::numeric_limits<unsigned>::max())
					throw std::invalid_argument("malformed input MMI file (core memory width of <MemoryLayout> exceeds implementation limits)");

				mapper m(static_cast<size_t>(mem_word_size));

				// Phase 2: Record the BRAM mappings
				//
				// TBD: It seems that the B data-port is also used (for example when synthesizing small 64-bit wide RAMs). The code below should
				// be generalized to allow for A/B port usage (bitlayout and MSB/LSB information seems to be accurate)
				for (auto xbram = xlayout->first_node("BRAM"); xbram; xbram = xbram->next_sibling("BRAM"))
				{
					// Extract the BRAM type and location
					auto& ram = resolve_bram(get_required_attribute(xbram, "MemType"), get_required_attribute(xbram, "Placement"));

					// Process port A
					{
						auto xdatawidth_a = xbram->first_node("DataWidth_PortA");
						auto xaddrrange_a = xbram->first_node("AddressRange_PortA");
						auto xbitlayout_a = xbram->first_node("BitLayout_PortA");
						parse_mem_array_dataport(m, ram, xdatawidth_a, xaddrrange_a, xbitlayout_a, mem_word_size);
					}

					// Process port B
					{
						auto xdatawidth_b = xbram->first_node("DataWidth_PortB");
						auto xaddrrange_b = xbram->first_node("AddressRange_PortB");
						auto xbitlayout_b = xbram->first_node("BitLayout_PortB");
						parse_mem_array_dataport(m, ram, xdatawidth_b, xaddrrange_b, xbitlayout_b, mem_word_size);
					}
				}

				std::cout << m << std::endl;
			}

			//-------------------------------------------------------------------------------------------------------------------
			std::vector<std::tuple<unsigned, bool>> mmi_parser_imp::parse_bitlayout(const std::string& bitlayout, const bram& ram)
			{
				std::vector<std::tuple<unsigned, bool>> layout;

				unsigned total_data_bits = 0u;
				unsigned total_parity_bits = 0u;
				unsigned field_width = 0u;

				bool is_parity = false;

				for (auto bp = bitlayout.begin(), be = bitlayout.end(); bp != be; ++bp)
				{
					// Consume the next character of the bit-layout
					auto type = *bp;

					if (type == 'p')
					{
						// Bits go to the parity bit
						if (field_width > 0u)
							throw std::invalid_argument("malformed input MMI file (missing separator in parity bit specification in <BitLayout_A> pattern)");

						is_parity = true;
					}
					else if (type == 'd')
					{
						// Bits go to the data bits
						if (field_width > 0u)
							throw std::invalid_argument("malformed input MMI file (missing separator in data bit specification in <BitLayout_A> pattern)");

						is_parity = false;
					}
					else if (type == '_')
					{
						// Separator (flush the current pattern)
						if (field_width > 0u)
						{
							// Assign some bits
							layout.emplace_back(std::make_tuple(field_width, is_parity));

							// Update the bit stats
							if (type)
							{
								// Partiy bit
								total_parity_bits += 1u;
							}
							else
							{
								// Data bit
								total_data_bits += 1u;
							}

							field_width = 0u;
						}
					}
					else if ('0' <= type && type <= '9')
					{
						// Next digit of width
						unsigned digit = static_cast<unsigned>(type - '0');

						if (field_width > (std::numeric_limits<unsigned>::max() / 10u))
							throw std::invalid_argument("malformed input MMI file (bit-width in <BitLayout_A> pattern exceeds implementation limits)");

						field_width *= 10u;

						if ((std::numeric_limits<unsigned>::max() - field_width) < digit)
							throw std::invalid_argument("malformed input MMI file (bit-width in <BitLayout_A> pattern exceeds implementation limits)");
						
						field_width += digit;
					}
					else
					{
						// Invalid bit layout specification
						throw std::invalid_argument("malformed input MMI file (unrecognized characters in <BitLayout_A> pattern)");
					}
				}

				// There should be one final pattern
				if (field_width > 0u)
				{
					// Assign some bits
					layout.emplace_back(std::make_tuple(field_width, is_parity));
				}

				// Final size check
				if (total_data_bits > ram.data_bits())
					throw std::invalid_argument("malformed input MMI file (number of data bits mapped in <BitLayout_A> pattern exceed BRAM limits)");

				if (total_parity_bits > ram.parity_bits())
					throw std::invalid_argument("malformed input MMI file (number of data bits mapped in <BitLayout_A> pattern exceed BRAM limits)");

				// We typically expect some extra bits (the sum of all field_widths can _at most_ be the word size; typically it will
				// be less for wide RAMs).
				return layout;
			}

			//-------------------------------------------------------------------------------------------------------------------
			const bram& mmi_parser_imp::resolve_bram(const std::string& type, const std::string& loc)
			{
				// Map to the correct RAM category
				bram_category category;
				{
					if (type == "RAMB36")
					{
						category = bram_category::ramb36;
					}
					else if (type == "RAMB18")
					{
						category = bram_category::ramb18;
					}
					else
					{
						throw std::invalid_argument("malformed input MMI file (implementation currently only supports RAMB36 and RAMB18)");
					}
				}

				// Parse the location string into X and Y coordinate.
				//
				// We have a fixed "X%uY%u" format. The sscanf() family of functions provides the easiest access
				unsigned loc_x = 0;
				unsigned loc_y = 0;

				
#if _MSC_VER > 0
				// We are on a MSVC compiler. Use sscanf_s() to silence CRT deprecation warnings.
				if (2 != ::sscanf_s(loc.c_str(), "X%uY%u", &loc_x, &loc_y))
					throw std::invalid_argument("malformed input MMI file (unparsable BRAM X/Y coordinates)");
#else
				// Not an MSVC compiler. Resort to plain old C sscanf()
				if (2 != std::sscanf(loc.c_str(), "X%uY%u", &loc_x, &loc_y))
					throw std::invalid_argument("malformed input MMI file (unparsable BRAM X/Y coordinates)");
#endif

				// Now resolve the BRAM
				return fpga->bram_by_loc(category, loc_x, loc_y);
			}

			//-------------------------------------------------------------------------------------------------------------------
			void mmi_parser_imp::parse_processor(const std::string& inst_path, xml_node* xprocessor)
			{
				// Phase 1: Find the total memory word size by scanning over the busblocks in all address spaces
				size_t mem_word_size = 0u;
				{
					for (auto xaddrspace = xprocessor->first_node("AddressSpace"); xaddrspace;
						xaddrspace = xaddrspace->next_sibling("AddressSpace"))
					{
						// TBD: Can there be more then 1 busblock per address space?
						auto xbusblock = xaddrspace->first_node("BusBlock");
						if (!xbusblock)
							throw std::invalid_argument("malformed input MMI file (failed to parse <AddressSpace> without <BusBlock>)");
					
						// TBD: Process the bit lanes
					}
				}

				if (!mem_word_size)
					throw std::invalid_argument("failed to infer memory word size from <Processor> tag in input MMI file");

				// Phase 2: Populate a mapping for the given address space
				{
					std::cerr << "UNIMPLEMENTED: parser Processor tag" << std::endl;
				}				
			}

			//-------------------------------------------------------------------------------------------------------------------
			void mmi_load(const std::string& filename, mmi_fpga_type_lookup_t fpga_lookup, mmi_instance_filter_t instance_filter)
			{
				if (!fpga_lookup)
					throw std::invalid_argument("FPGA type lookup helper cannot be a null pointer");

				mmi_parser_imp parser(filename, fpga_lookup, instance_filter);
			}
		}
	}
}