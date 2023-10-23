/**
 * @file
 * @brief libxml2 C++ wrappers
 */
#define UNBIT_XML_IMPLEMENTATION 1
#include "xml/xml.hpp"

namespace unbit
{
	namespace xml
	{
		const xmlChar* to_xml_char(const char *str)
		{
			// We assume UTF-8 (or C locale) for simplicity (at the moment).
			return reinterpret_cast<const xmlChar *>(str);
		}

		const char* from_xml_char(const xmlChar *str)
		{
			// We assume UTF-8 (or C locale) for simplicity (at the moment).
			return reinterpret_cast<const char *>(str);
		}
	}
}
