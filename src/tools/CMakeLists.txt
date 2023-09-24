
LINK_LIBRARIES(unbit_xilinx)

ADD_EXECUTABLE(zynq7-unbit-dump-bitstream         unbit-dump-bitstream.cpp)
ADD_EXECUTABLE(zynq7-unbit-dump-brams             unbit-dump-brams.cpp)
ADD_EXECUTABLE(zynq7-unbit-substitute-brams       unbit-substitute-brams.cpp)

ADD_EXECUTABLE(vup-unbit-dump-bitstream           unbit-dump-bitstream.cpp)
ADD_EXECUTABLE(vup-unbit-dump-brams               unbit-dump-brams.cpp)
ADD_EXECUTABLE(vup-unbit-substitute-brams         unbit-substitute-brams.cpp)
TARGET_COMPILE_OPTIONS(vup-unbit-dump-brams       PUBLIC -DEXPERIMENTAL_VIRTEX_UP)
TARGET_COMPILE_OPTIONS(vup-unbit-substitute-brams PUBLIC -DEXPERIMENTAL_VIRTEX_UP)
