#
# XML support (libxml2 wrappers)
#
IF (LIBXML2_FOUND)
	ADD_LIBRARY(unbit_xml STATIC)

	TARGET_SOURCES(unbit_xml
		PUBLIC
			FILE_SET HEADERS
			BASE_DIRS
				${UNBIT_INCLUDE_DIR}
			FILES
				${UNBIT_INCLUDE_DIR}/unbit/xml/xml.hpp

		PRIVATE
			xml.cpp
			xml_doc.cpp
			xml_node.cpp
			xml_parser_guard.cpp
			xml_string.cpp
			xpath_context.cpp
			xpath_result.cpp
	)

	TARGET_INCLUDE_DIRECTORIES(unbit_xml
		PRIVATE ${LIBXML2_INCLUDE_DIR}
	)

	TARGET_LINK_LIBRARIES(unbit_xml 
		INTERFACE ${LIBXML2_LIBRARY}
	)


	INSTALL(
		TARGETS
			unbit_xml
		EXPORT UnbitXml
		RUNTIME
			COMPONENT Runtime
		LIBRARY
			COMPONENT Runtime
		ARCHIVE
			COMPONENT Development
		FILE_SET HEADERS
			COMPONENT Development
	)

	INSTALL(
		EXPORT UnbitXml
		DESTINATION lib/cmake
	)
ENDIF ()
