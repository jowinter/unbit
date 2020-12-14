CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

PROJECT(unbit)

# Language standard is C++17 (with pedantic warnings)
ADD_DEFINITIONS(-Wall -pedantic -std=c++17)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/inc")

ADD_SUBDIRECTORY(src)
