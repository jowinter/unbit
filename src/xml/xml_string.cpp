/**
 * @file
 * @brief libxml2 C++ wrappers (XML string support)
 */
#define UNBIT_XML_IMPLEMENTATION 1
#include "unbit/xml/xml.hpp"

#include <libxml/xmlstring.h>

namespace unbit
{
	namespace xml
	{
		//-------------------------------------------------------------------------------------
		xml_string::xml_string(xmlChar* str)
			: str_(str)
		{
		}

		//-------------------------------------------------------------------------------------
		xml_string::~xml_string()
		{
			if (str_)
			{
				xmlFree(str_);
				str_ = nullptr;
			}
		}

		//-------------------------------------------------------------------------------------
		xml_string::operator const char*() const
		{
			return str_ ? from_xml_char(str_) : "";
		}

		//-------------------------------------------------------------------------------------
		bool xml_string::operator==(const char *other) const
		{
			return (0 == xmlStrcmp(str_, to_xml_char(other)));
		}

		//-------------------------------------------------------------------------------------
		bool xml_string::operator!=(const char *other) const
		{
			return (0 != xmlStrcmp(str_, to_xml_char(other)));
		}

		//-------------------------------------------------------------------------------------
		bool xml_string::operator==(const xml_string& other) const
		{
			return (0 == xmlStrcmp(str_, other.str_));
		}

		//-------------------------------------------------------------------------------------
		bool xml_string::operator!=(const xml_string& other) const
		{
			return (0 != xmlStrcmp(str_, other.str_));
		}
	}
}
