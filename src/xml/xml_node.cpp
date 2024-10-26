/**
 * @file
 * @brief libxml2 C++ wrappers
 */
#define UNBIT_XML_IMPLEMENTATION 1
#include "unbit/xml/xml.hpp"

#include <cstdlib>
#include <errno.h>
#include <stdexcept>

namespace unbit
{
	namespace xml
	{
		//-------------------------------------------------------------------------------------
		xml_node::xml_node(xmlNodePtr node)
			: node_(node)
		{
			if (!node)
			{
				throw std::invalid_argument("invalid xml node object (null)");
			}
		}

		//-------------------------------------------------------------------------------------
		xml_node::~xml_node()
		{
		}

		//-------------------------------------------------------------------------------------
		xmlNodePtr xml_node::get()
		{
			return node_;
		}

		//-------------------------------------------------------------------------------------
		xml_string xml_node::attribute(const char* name)
		{
			return xml_string(xmlGetProp(node_, to_xml_char(name)));
		}

		//-------------------------------------------------------------------------------------
		uint64_t xml_node::attribute_as_uint64(const char *name, uint64_t def_value)
		{
			auto attr = attribute(name);
			uint64_t result = def_value;

			if (*attr != '\0')
			{
				// Attribute value given
				char *endp = NULL;

				errno = 0;
				result = strtoull(attr, &endp, 0);
				if (*endp != '\0' || errno != 0)
				{
					throw std::runtime_error("failed to parse an attribute value as 64-bit unsigned integer");
				}
			}

			return result;
		}
	}
}
