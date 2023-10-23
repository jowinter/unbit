/**
 * @file
 * @brief libxml2 C++ wrappers (init/cleanup guard)
 */
#define UNBIT_XML_IMPLEMENTATION 1
#include "xml/xml.hpp"

namespace unbit
{
	namespace xml
	{
		//-----------------------------------------------------------------------------------------
		xml_parser_guard::xml_parser_guard()
		{
			xmlInitParser();
		}

		//-----------------------------------------------------------------------------------------
		xml_parser_guard::~xml_parser_guard()
		{
			xmlCleanupParser();
		}
	}
}
