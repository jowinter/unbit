# XML support (optional; requires libxml2)
ADD_SUBDIRECTORY(xml)

# Intel-Hex support
ADD_SUBDIRECTORY(ihex)

# Build FPGA bitstream manipulation libraries
ADD_SUBDIRECTORY(fpga)

# And build the standalone tools
ADD_SUBDIRECTORY(tools)
