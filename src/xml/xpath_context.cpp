/**
 * @file
 * @brief libxml2 C++ wrappers (XML document wrapper)
 */
#define UNBIT_XML_IMPLEMENTATION 1
#include "unbit/xml/xml.hpp"

namespace unbit
{
	namespace xml
	{
		//-----------------------------------------------------------------------------------------
		xpath_context::xpath_context(xml_doc& doc)
			: ctx_(xmlXPathNewContext(doc.get()))
		{
			if (!ctx_)
			{
				throw std::runtime_error("failed to create an xpath context");
			}
		}

		//-----------------------------------------------------------------------------------------
		xpath_context::~xpath_context()
		{
			xmlXPathFreeContext(ctx_);
			ctx_ = nullptr;
		}

		//-----------------------------------------------------------------------------------------
		xmlXPathContextPtr xpath_context::get()
		{
			return ctx_;
		}

		//-----------------------------------------------------------------------------------------
		xpath_context::result xpath_context::query(const std::string& expr)
		{
			return result(*this, expr);
		}

		//-----------------------------------------------------------------------------------------
		xpath_context::result xpath_context::query(xml_node& node, const std::string& expr)
		{
			return result(*this, node, expr);
		}
	}
}
