# XML support (optional; requires libxml2)
ADD_SUBDIRECTORY(xml)

# Build FPGA bitstream manipulation libraries
ADD_SUBDIRECTORY(fpga)

# And build the standalone tools
ADD_SUBDIRECTORY(tools)
