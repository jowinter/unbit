#
# XML support (libxml2 wrappers)
#
IF (LIBXML2_FOUND)
  ADD_LIBRARY(unbit_xml
	STATIC
	xml.cpp
	xml_doc.cpp
	xml_node.cpp
	xml_parser_guard.cpp
	xml_string.cpp
	xpath_context.cpp
	xpath_result.cpp)
  TARGET_INCLUDE_DIRECTORIES(unbit_xml PUBLIC "${CMAKE_SOURCE_DIR}/inc" "${LIBXML2_INCLUDE_DIR}")
  TARGET_LINK_LIBRARIES(unbit_xml PUBLIC "${LIBXML2_LIBRARY}")
ENDIF ()
