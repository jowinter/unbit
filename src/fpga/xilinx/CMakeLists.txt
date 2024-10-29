# Legacy functions
IF (UNBIT_ENABLE_LEGACY)
	ADD_SUBDIRECTORY(old)
ENDIF ()


# The new unbit library
ADD_LIBRARY(unbit_xilinx STATIC)

TARGET_SOURCES(unbit_xilinx
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
			${UNBIT_INCLUDE_DIR}
		FILES
			${UNBIT_INCLUDE_DIR}/unbit/fpga/xilinx/bitstream_engine.hpp
			${UNBIT_INCLUDE_DIR}/unbit/fpga/xilinx/bitstream_error.hpp
			${UNBIT_INCLUDE_DIR}/unbit/fpga/xilinx/config_cmd.hpp
			${UNBIT_INCLUDE_DIR}/unbit/fpga/xilinx/config_context.hpp
			${UNBIT_INCLUDE_DIR}/unbit/fpga/xilinx/config_engine.hpp
			${UNBIT_INCLUDE_DIR}/unbit/fpga/xilinx/config_reg.hpp

	PRIVATE
		bitstream_engine.cpp
		bitstream_error.cpp
		config_cmd.cpp
		config_context.cpp
		config_engine.cpp
		config_reg.cpp
)

INSTALL(
	TARGETS
		unbit_xilinx
	EXPORT UnbitXilinx
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
	EXPORT UnbitXilinx
	DESTINATION lib/cmake
)
