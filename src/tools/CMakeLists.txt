ADD_SUBDIRECTORY(old)

ADD_EXECUTABLE(unbit-analyze
	unbit-analyze.cpp
)

TARGET_LINK_LIBRARIES(unbit-analyze
	PRIVATE
		unbit_xilinx
)