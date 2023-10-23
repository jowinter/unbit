/**
 * @file
 * @brief Parser support for Xilinx Memory Map Information (MMI) files
 */
#include "mmi_detail.hpp"

using unbit::xml::xml_doc;
using unbit::xml::xml_node;
using unbit::xml::xpath_context;

namespace unbit
{
	namespace xilinx
	{
		namespace mmi
		{
			//-------------------------------------------------------------------------------------
			static auto get_processor_node(xml_doc& xdoc, const std::string& instance)
			{
				auto xnode = xpath_context(xdoc).query("/MemInfo/Processor[@InstPath=\"" +
													   instance + "]");
				if (0u == xnode.node_count())
				{
					std::string msg = "failed to locate processor instance '" +
						instance + "' in mmi file";
					throw std::runtime_error(msg);
				}

				return xnode.node_at(0u);
			}

			//-------------------------------------------------------------------------------------
			std::unique_ptr<memory_map> memory_map::load(const std::string& filename,
														 const std::string& instance)
			{
				xml_doc xdoc(filename);

				// Locate the processor node and get its endianness
				auto xproc = get_processor_node(xdoc, instance);
				return std::make_unique<cpu_memory_map>(xdoc, xproc);
			}

			//-------------------------------------------------------------------------------------
			memory_map::memory_map()
			{
			}

			//-------------------------------------------------------------------------------------
			memory_map::~memory_map()
			{
			}

			//-------------------------------------------------------------------------------------
			uint8_t memory_map::read_byte(const fpga& fpga, const bitstream& bs,
										  uint64_t byte_addr) const
			{
				uint8_t value = 0u;
				for (size_t i = 0u; i < 8u; ++i)
				{
					value |= ((uint8_t) read_bit(fpga, bs, byte_addr * 8u + i)) << i;
				}

				return value;
			}
		}
	}
}
