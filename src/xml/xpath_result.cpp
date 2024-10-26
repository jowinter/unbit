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
		xpath_context::result::result(xpath_context& ctx, const char *expr)
			: obj_(xmlXPathEvalExpression(to_xml_char(expr), ctx.get()))
		{
			if (!obj_)
			{
				throw std::runtime_error("failed to evaluate xpath expression");
			}
		}

		//-----------------------------------------------------------------------------------------
		xpath_context::result::result(xpath_context& ctx, const std::string& expr)
			: result(ctx, expr.c_str())
		{
		}

		//-----------------------------------------------------------------------------------------
		xpath_context::result::result(xpath_context& ctx, xml_node& node, const char *expr)
			: obj_(xmlXPathNodeEval(node.get(), to_xml_char(expr), ctx.get()))
		{
			if (!obj_)
			{
				throw std::runtime_error("failed to evaluate xpath expression");
			}
		}

		//-----------------------------------------------------------------------------------------
		xpath_context::result::result(xpath_context& ctx, xml_node& node, const std::string& expr)
			: result(ctx, node, expr.c_str())
		{
		}

		//-----------------------------------------------------------------------------------------
		xpath_context::result::~result()
		{
			xmlXPathFreeObject(obj_);
			obj_ = nullptr;
		}

		//-----------------------------------------------------------------------------------------
		size_t xpath_context::result::node_count()
		{
			type_check_node();

			return xmlXPathNodeSetGetLength(obj_->nodesetval);
		}

		//-----------------------------------------------------------------------------------------
		xml_node xpath_context::result::node_at(unsigned index)
		{
			type_check_node();

			return xml_node(xmlXPathNodeSetItem(obj_->nodesetval, static_cast<int>(index)));
		}

		//-----------------------------------------------------------------------------------------
		void xpath_context::result::type_check_node()
		{
			if (obj_->type != XPATH_NODESET)
			{
				throw std::runtime_error("xpath result type mismatch");
			}
		}
	}
}
