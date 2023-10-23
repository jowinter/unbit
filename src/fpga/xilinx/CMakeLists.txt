#
# Build the static "libunbit-xilinx7" library (containing all reusable components)
#

ADD_LIBRARY(unbit_xilinx STATIC
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

TARGET_INCLUDE_DIRECTORIES(unbit_xilinx PRIVATE "${PROJECT_SOURCE_DIR}/external")

IF (UNBIT_ENABLE_MMI)
  TARGET_SOURCES(unbit_xilinx        PRIVATE mmi.cpp mmi_cpu_memory_map.cpp mmi_cpu_memory_region.cpp)
  TARGET_LINK_LIBRARIES(unbit_xilinx PRIVATE unbit_xml)
ENDIF ()
