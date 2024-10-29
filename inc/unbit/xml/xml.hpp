/**
 * @file
 * @brief libxml2 C++ wrappers
 */
#ifndef UNBIT_XML_HPP_
#define UNBIT_XML_HPP_ 1

#include <memory>
#include <stdexcept>
#include <string>

#if !defined(UNBIT_XML_IMPLEMENTATION) || (UNBIT_XML_IMPLEMENTATION == 0)
// Provide libxml2 compatible stub definitions (don't include libxml2 headers)

/** @brief Stub definition for libxml2 @c xmlChar type*/
typedef unsigned char                xmlChar;

/** @brief Stub definition for libxml2 @c xmlDocPtr type*/
typedef struct _xmlDocType*          xmlDocPtr;

/** @brief Stub definition for libxml2 @c xmlNodePtr type*/
typedef struct _xmlNodeType*         xmlNodePtr;

/** @brief Stub definition for libxml2 @c xmlXPathContextPtr type*/
typedef struct _xmlXPathContextType* xmlXPathContextPtr;

/** @brief Stub definition for libxml2 @c xmlXPathObjectPtr type*/
typedef struct _xmlXPathObjectType*  xmlXPathObjectPtr;

# else
// Pull-in implementation details (libxml2 headers)
# include <libxml/parser.h>
# include <libxml/tree.h>
# include <libxml/xpath.h>

#endif

namespace unbit
{
	namespace xml
	{
		//-------------------------------------------------------------------------------------
		/**
		 * @brief Guard object for XML (libxml2) parser initialization and cleanup.
		 *
		 * This helper class encapsulates global initilaization and teardown of the
		 * libxml2 library state. It can be used at level of the program's main routine
		 * to ensure full resource deallocation of libxml2's global state before leaving
		 * the program.
		 */
		class xml_parser_guard
		{
		public:
			/**
			 * @brief Allocates the global state of the XML parser library.
			 */
			xml_parser_guard();

			/**
			 * @brief Deallocaes the global state of the XML parser library.
			 */
			~xml_parser_guard();

		protected:
			// Non-copyable
			xml_parser_guard(const xml_parser_guard& other) =delete;
			xml_parser_guard& operator= (const xml_parser_guard& other) =delete;
		};

		//-------------------------------------------------------------------------------------
		/**
		 * @brief Conversion helper from C strings to xmlChar strings.
		 */
		extern const xmlChar* to_xml_char(const char *str);

		//-------------------------------------------------------------------------------------
		/**
		 * @brief Conversion helper from xmlChar strings to xmlChar constants.
		 */
		extern const char* from_xml_char(const xmlChar *str);

		//-------------------------------------------------------------------------------------
		/**
		 * @brief Thin C++ wrapper around libxml2 allocate strings.
		 */
		class xml_string
		{
		private:
			/**
			 * @brief The underlying XML string data (alloctaed by libxml2)
			 */
			xmlChar* str_;

		public:
			/**
			 * @brief Constructs a new XML string wrapper
			 */
			explicit xml_string(xmlChar* str);

			/**
			 * @brief Destroys an XML string wrapper.
			 */
			~xml_string();

			/**
			 * @brief Implicit conversion to a C string.
			 *
			 * This method assumes that the caller is capable of handling UTF-8
			 * encoded strings. NULL strings are implicitly converted to the empty
			 * string (for simplicity).
			 */
			operator const char*() const;

			/**
			 * @brief Compares this XML string and a given C string for equality.
			 */
			bool operator==(const char *other) const;

			/**
			 * @brief Compares this XML string and a given C string for equality.
			 */
			bool operator!=(const char *other) const;

			/**
			 * @brief Compares two XML strings for equality.
			 */
			bool operator==(const xml_string& other) const;

			/**
			 * @brief Compares two XML strings for inequality.
			 */
			bool operator!=(const xml_string& other) const;

		private:
			// Non-copyable
			xml_string(const xml_string&) =delete;
			xml_string& operator=(const xml_string&) =delete;
		};

		//-------------------------------------------------------------------------------------
		/**
		 * @brief Thin C++ wrapper around libxml2's XML node interface.
		 */
		class xml_node
		{
		private:
			/**
			 * @brief XML node implementation (libxml2)
			 */
			xmlNodePtr node_;

		public:
			/**
			 * @brief Constructs an XML node wrapper from the low-level implementation.
			 *
			 * @param[in] node is the node implementation to be wrapped.
			 */
			explicit xml_node(xmlNodePtr node);

			/**
			 * @brief Disposes an XML node wrapper
			 */
			~xml_node();

			/**
			 * @brief Gets the low-level XML node implementation
			 */
			xmlNodePtr get();

			/**
			 * @brief Gets an attribute of the selected node as string.
			 *
			 * @param[in] name is the name of the attribute to query.
			 *
			 * @return The attribute value (empty/missing attributes are implicitly mapped to
			 *   the empty string).
			 */
			xml_string attribute(const char* name);

			/**
			 * @brief Gets a attribute of the selected node as 64-bit integer value.
			 *
			 * @param[in] name is the name of the attribute to query.
			 *
			 * @param[in] def_value is the default value (for empty/missing attributes).
			 *
			 * @return The attribute value (empty/missing attributes are implicitly mapped to
			 *   the given default value).
			 *
			 * @throw runtime_error if parsing of the attribute value fails.
			 */
			uint64_t attribute_as_uint64(const char *name, uint64_t def_value = 0u);
		};

		//-------------------------------------------------------------------------------------
		/**
		 * @brief Thin C++ wrapper around libxml2's reader interface.
		 */
		class xml_doc
		{
		private:
			/**
			 * @brief XML document implementation (libxml2)
			 */
			xmlDocPtr doc_;

		public:
			/**
			 * @brief Constructs an XML document from a given file
			 */
			xml_doc(const std::string& filename);

			/**
			 * @brief Disposes an XML document
			 */
			~xml_doc();

			/**
			 * @brief Gets the XML document implemention
			 */
			xmlDocPtr get();

		private:
			// Non-copyable
			xml_doc(const xml_doc& other) =delete;
			xml_doc& operator=(const xml_doc& other) =delete;
		};

		//-------------------------------------------------------------------------------------
		/**
		 * @brief Thin C++ wrapper around libxml2's xpath interface.
		 */
		class xpath_context
		{
		private:
			/**
			 * @brief Pointer to the underlying XPath context implementation.
			 */
			xmlXPathContextPtr ctx_;

		public:
			/**
			 * @brief XPath result set
			 */
			class result
			{
			private:
				/**
				 * @brief XPath results set implementation
				 */
				xmlXPathObjectPtr obj_;

			public:
				/**
				 * @brief Evaluates an XPath query constructing a result set.
				 */
				result(xpath_context& ctx, const char *expr);

				/**
				 * @brief Evaluates an XPath query constructing a result set.
				 */
				result(xpath_context& ctx, const std::string& expr);

				/**
				 * @brief Evaluates an XPath query (with context node) constructing a result set.
				 */
				result(xpath_context& ctx, xml_node& node, const char *expr);

				/**
				 * @brief Evaluates an XPath query (with context node) constructing a result set.
				 */
				result(xpath_context& ctx, xml_node& node, const std::string& expr);

				/**
				 * @brief Disposes an XPath query result.
				 */
				~result();

				/**
				 * @brief Gets the number of XML nodes in the returned node set.
				 */
				size_t node_count();

				/**
				 * Gets an XML node at a specified index.
				 */
				xml_node node_at(unsigned index);

			protected:
				/**
				 * @brief Type-check assertion on the result set type (must be a nodeset)
				 */
				void type_check_node();

			private:
				// Non-copyable
				result(const result&) =delete;
				result& operator=(const result&) =delete;
			};

		public:
			/**
			 * @brief Constructs a new XPath context
			 */
			xpath_context(xml_doc& doc);

			/**
			 * @brief Disposes an XPath context
			 */
			~xpath_context();

			/**
			 * @brief Gets the XPath context implementation.
			 */
			xmlXPathContextPtr get();

			/**
			 * @brief Queries the first XML element matching a given expression
			 */
			result query(const std::string& expr);

			/**
			 * @brief Queries the first XML element matching a given expression
			 *   (with respect to a context node)
			 */
			result query(xml_node& node, const std::string& expr);

		private:
			// Non-copyable
			xpath_context(const xpath_context&) =delete;
			xpath_context& operator=(const xpath_context&) =delete;
		};
	}
}

#endif // #ifndef UNBIT_XML_HPP_
