CMAKE_MINIMUM_REQUIRED(VERSION 3.30)
PROJECT(unbit VERSION 0.1 LANGUAGES C CXX)

# Request C++20
SET(CMAKE_CXX_STANDARD 20)

OPTION (UNBIT_ENABLE_LEGACY "Enable old (legacy) unbit tooling" FALSE)

FIND_PACKAGE(LibXml2)

# Include directory (for subprojects)
SET (UNBIT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# Build projects in subdirectories
ADD_SUBDIRECTORY(src)

#
# Install setup (legacy tooling)
#
IF (UNBIT_ENABLE_LEGACY)
	INSTALL(DIRECTORY inc/ DESTINATION include/unbit FILES_MATCHING PATTERN "*.hpp")
	INSTALL(
		TARGETS 
			unbit-old-dump-bitstream 
			unbit-old-dump-brams 
			unbit-old-substitute-brams
			unbit-old-strip-crc-checks
			unbit_xilinx_old
			unbit_ihex

		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)

	IF (UNBIT_ENABLE_MMI)
		INSTALL(
			TARGETS 
				unbit-old-dump-image
				unbit-old-inject-image
				unbit_xml
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)
	ENDIF()
ENDIF ()

#
# Packaging
#
SET(CPACK_GENERATOR        "TGZ")
SET(CPACK_SOURCE_GENERATOR "TGZ")

INCLUDE(CPack)
