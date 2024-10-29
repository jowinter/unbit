#
# Build the static "libunbit-xilinx7" library (containing all reusable components)
#

OPTION (UNBIT_ENABLE_MMI "Enable (experimental) support for Xilinx Memory Map Information (MMI) files (requires libxml2)" TRUE)

IF (UNBIT_ENABLE_MMI)
  # libxml2 support (needed for parsing Xilinx MMI files)
  IF (NOT LIBXML2_FOUND)
	  MESSAGE(ERROR "Xilinx Memory Map Information (MMI) support requires libxml2")
  ENDIF()
ENDIF()

ADD_LIBRARY(unbit_xilinx_old STATIC
  bitstream.cpp
  bram.cpp
  ramb36e1.cpp
  ramb18e1.cpp
  ramb36e2.cpp
  fpga.cpp

  v7/zynq7.cpp
  v7/xc7z010.cpp
  v7/xc7z015.cpp
  v7/xc7z020.cpp

  vup/virtex_up.cpp
  vup/xcvu9p.cpp)

TARGET_INCLUDE_DIRECTORIES(unbit_xilinx_old PRIVATE "${PROJECT_SOURCE_DIR}/external")

IF (UNBIT_ENABLE_MMI)
  TARGET_SOURCES(unbit_xilinx_old        PRIVATE mmi.cpp mmi_cpu_memory_map.cpp mmi_cpu_memory_region.cpp)
  TARGET_LINK_LIBRARIES(unbit_xilinx_old PRIVATE unbit_xml)
ENDIF ()
