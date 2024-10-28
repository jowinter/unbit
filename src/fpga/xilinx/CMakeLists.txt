# Legacy functions
ADD_SUBDIRECTORY(old)


# The new unbit library
ADD_LIBRARY(unbit_xilinx OBJECT
	bitstream_engine.cpp
	bitstream_error.cpp
	config_cmd.cpp
	config_context.cpp
	config_engine.cpp
	config_reg.cpp
	
)
