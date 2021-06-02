CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

PROJECT(unbit VERSION 0.1 LANGUAGES CXX)

#
# Language standard is C++17, all warnings are enabled.
#
IF (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
 #
 # Visual Studio (either native MSVC or clang-cl)
 #
 MESSAGE(STATUS "Using Visual Studio build configuation")
 ADD_DEFINITIONS(/W3 /std:c++17)

ELSEIF (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
 #
 # GCC/Clang compiler toolchain
 #
 MESSAGE(STATUS "Using Clang/GCC build configuration")
 ADD_DEFINITIONS(-Wall -pedantic -std=c++17)

ELSE ()
 # Not yet support
 MESSAGE(WARNING "Unsupported compiler configuration (expecting trouble); compiler-id: ${CMAKE_CXX_COMPILER_ID}")
ENDIF ()

# Common include tree for the entire project
INCLUDE_DIRECTORIES(
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inc>
	$<INSTALL_INTERFACE:include/unbit>
)

# Build projects in subdirectories
ADD_SUBDIRECTORY(src)

#
# Install setup
#
INSTALL(DIRECTORY inc/ DESTINATION include/unbit FILES_MATCHING PATTERN "*.hpp")
INSTALL(TARGETS unbit-dump-brams unbit_xilinx
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
