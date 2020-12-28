/**
 * @file
 * @brief Xilinx BRAM Memory Map Info (MMI) File manipulation
 */
#include "fpga/xilinx/mapper.hpp"
#include "fpga/xilinx/bram.hpp"
#include "fpga/xilinx/mmi.hpp"

#include "rapidxml/rapidxml.hpp"

#include <iostream>
#include <fstream>

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
				static std::string get_attribute(xml_node* node, const char* name);

				void parse_rams(mmi_instance_filter_t instance_filter);
				void parse_mem_array(const std::string& inst_path, xml_node* xmemarray);
				void parse_processor(const std::string& inst_path, xml_node* xprocessor);

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
						auto name = get_attribute(xoption, "Name");
						auto value = get_attribute(xoption, "Val");

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
			std::string mmi_parser_imp::get_attribute(xml_node* node, const char* name)
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
					// Check the instance path
					auto inst_path = get_attribute(xnode, "InstPath");
					if (!inst_path.empty() && instance_filter(inst_path))
					{
						// Non-empty instance path, filter accepted
						auto tag_name = xnode->name();

						if (mem_array_tag == tag_name)
						{
							// MemoryArray tag describing an XPM memory macro
							parse_mem_array(inst_path, xnode);
						}
						else if (processor_tag == tag_name)
						{
							// Processor tag describing an embedded processor address space.
							parse_processor(inst_path, xnode);
						}
						else
						{
							// Other memory tag (cuurrenly ignored - TBD: throw?)
						}
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

				// Phase 1: Scan over the BRAM tags to determine the memory word size (TBD: we only look at PortA) and collect placement data
				size_t mem_word_size = 0u;
				{
					for (auto xbram = xlayout->first_node("BRAM"); xbram; xbram = xlayout->next_sibling("BRAM"))
					{
						auto xdatawidth_a = xbram->first_node("DataWidth_PortA");
						if (!xdatawidth_a)
							throw std::invalid_argument("malformed input MMI file (failed to parse <MemoryLayout> without <DataWidth_PortA>)");

						auto msb = get_attribute(xdatawidth_a, "MSB");
						auto lsb = get_attribute(xdatawidth_a, "LSB");
						std::cout << msb << "..." << lsb << std::endl;
					}
				}

				std::cerr << "UNIMPLEMENTED: parser MemoryArray tag" << std::endl;
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