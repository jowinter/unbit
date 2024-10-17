LINK_LIBRARIES(unbit_xilinx_old)

ADD_EXECUTABLE(unbit-old-dump-bitstream         unbit-dump-bitstream.cpp)
ADD_EXECUTABLE(unbit-old-dump-brams             unbit-dump-brams.cpp)
ADD_EXECUTABLE(unbit-old-substitute-brams       unbit-substitute-brams.cpp)
ADD_EXECUTABLE(unbit-old-strip-crc-checks       unbit-strip-crc-checks.cpp)
ADD_EXECUTABLE(unbit-old-bitstream-to-readback  unbit-bitstream-to-readback.cpp)

IF (UNBIT_ENABLE_MMI)
  ADD_EXECUTABLE(unbit-old-dump-image           unbit-dump-image.cpp)

  ADD_EXECUTABLE(unbit-old-inject-image         unbit-inject-image.cpp)
  TARGET_LINK_LIBRARIES(unbit-old-inject-image  PRIVATE unbit_ihex)
ENDIF ()
