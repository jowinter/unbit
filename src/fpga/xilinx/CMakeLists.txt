# Legacy functions
ADD_SUBDIRECTORY(old)


# The new unbit library
ADD_LIBRARY(unbit_xilinx OBJECT
	bitstream_engine.cpp
)
