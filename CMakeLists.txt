CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

PROJECT(unbit VERSION 0.1 LANGUAGES CXX)

#
# Language standard is C++17, all warnings are enabled.
#
IF (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
 #
 # Visual Studio
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

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/inc")

ADD_SUBDIRECTORY(src)
