/**
 * @file
 * @brief Detail implementation of support for Xilinx Memory Map Information (MMI) files.
 */
#include "mmi_detail.hpp"

#include <cstdio>
#include <tuple>

#include <iostream>

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
			cpu_memory_map::mmi_space::mmi_space()
				: start_byte_addr(0u), end_byte_addr(0u), total_num_words(0u), word_size(0u)
			{
			}

			//-------------------------------------------------------------------------------------
			cpu_memory_map::mmi_space::~mmi_space()
			{
			}

			//-------------------------------------------------------------------------------------
			const std::string& cpu_memory_map::mmi_space::name() const
			{
				return region_name;
			}

			//-------------------------------------------------------------------------------------
			uint64_t cpu_memory_map::mmi_space::start_bit_addr() const
			{
				return start_byte_addr * 8u;
			}

			//-------------------------------------------------------------------------------------
			uint64_t cpu_memory_map::mmi_space::end_bit_addr() const
			{
				return end_byte_addr * 8u;
			}
		}
	}
}
