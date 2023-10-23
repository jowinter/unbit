LINK_LIBRARIES(unbit_xilinx)

ADD_EXECUTABLE(unbit-dump-bitstream         unbit-dump-bitstream.cpp)
ADD_EXECUTABLE(unbit-dump-brams             unbit-dump-brams.cpp)
ADD_EXECUTABLE(unbit-substitute-brams       unbit-substitute-brams.cpp)
ADD_EXECUTABLE(unbit-strip-crc-checks       unbit-strip-crc-checks.cpp)
ADD_EXECUTABLE(unbit-bitstream-to-readback  unbit-bitstream-to-readback.cpp)

IF (UNBIT_ENABLE_MMI)
  ADD_EXECUTABLE(unbit-dump-image           unbit-dump-image.cpp)
ENDIF ()
