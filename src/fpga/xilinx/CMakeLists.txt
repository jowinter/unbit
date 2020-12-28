#
# Build the static "libunbit-xilinx7" library (containing all reusable components)
#

ADD_LIBRARY(unbit_xilinx STATIC
  v7/bitstream.cpp
  v7/bitlayout.cpp
  v7/bram.cpp
  v7/ramb36e1.cpp
  v7/zynq7.cpp
  v7/xc7z020.cpp
  v7/mapper.cpp
  v7/mmi.cpp
)

TARGET_INCLUDE_DIRECTORIES(unbit_xilinx PRIVATE "${PROJECT_SOURCE_DIR}/external")