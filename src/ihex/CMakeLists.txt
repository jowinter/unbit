#
# Intel-Hex support library
#
ADD_LIBRARY(unbit_ihex STATIC)

TARGET_SOURCES(unbit_ihex
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
			${UNBIT_INCLUDE_DIR}

		FILES
			${UNBIT_INCLUDE_DIR}/unbit/ihex/ihex.hpp

	PRIVATE
		ihex.cpp
)


INSTALL(
	TARGETS
		unbit_ihex
	EXPORT UnbitIntelHex
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
	EXPORT UnbitIntelHex
	DESTINATION lib/cmake
)
