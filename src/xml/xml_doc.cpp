/**
 * @file
 * @brief libxml2 C++ wrappers (XML document wrapper)
 */
#define UNBIT_XML_IMPLEMENTATION 1
#include "xml/xml.hpp"

namespace unbit
{
	namespace xml
	{
		//-----------------------------------------------------------------------------------------
		xml_doc::xml_doc(const std::string& filename)
			: doc_(xmlParseFile(filename.c_str()))
		{
			if (!doc_)
			{
				throw std::runtime_error("failed to parse xml document");
			}
		}

		//-----------------------------------------------------------------------------------------
		xml_doc::~xml_doc()
		{
			xmlFreeDoc(doc_);
			doc_ = nullptr;
		}

		//-----------------------------------------------------------------------------------------
		xmlDocPtr xml_doc::get()
		{
			return doc_;
		}
	}
}
