CMAKE_MINIMUM_REQUIRED(VERSION 3.30)
PROJECT(unbit VERSION 0.1 LANGUAGES C CXX)

# Request C++20
SET(CMAKE_CXX_STANDARD 20)

# Common include tree for the entire project
INCLUDE_DIRECTORIES(
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inc>
	$<INSTALL_INTERFACE:include>
)

OPTION (UNBIT_ENABLE_MMI "Enable (experimental) support for Xilinx Memory Map Information (MMI) files (requires libxml2)" TRUE)

IF (UNBIT_ENABLE_MMI)
  # libxml2 support (needed for parsing Xilinx MMI files)
  FIND_PACKAGE(LibXml2)

  IF (NOT LIBXML2_FOUND)
	MESSAGE(ERROR "Xilinx Memory Map Information (MMI) support requires libxml2")
  ENDIF()
ENDIF()

# Build projects in subdirectories
ADD_SUBDIRECTORY(src)

#
# Install setup
#
INSTALL(DIRECTORY inc/ DESTINATION include/unbit FILES_MATCHING PATTERN "*.hpp")
INSTALL(TARGETS unbit-old-dump-bitstream unbit-old-dump-brams unbit-old-substitute-brams unbit-old-strip-crc-checks unbit_xilinx_old unbit_ihex
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

IF (UNBIT_ENABLE_MMI)
INSTALL(TARGETS unbit-old-dump-image unbit-old-inject-image unbit_xml
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
ENDIF()

#
# Packaging
#
SET(CPACK_GENERATOR        "TGZ")
SET(CPACK_SOURCE_GENERATOR "TGZ")

INCLUDE(CPack)
