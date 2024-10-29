IF (UNBIT_ENABLE_LEGACY)
	ADD_SUBDIRECTORY(old)
ENDIF ()

ADD_EXECUTABLE(unbit-analyze
	unbit-analyze.cpp
)

TARGET_LINK_LIBRARIES(unbit-analyze
	PRIVATE
		unbit_xilinx
)

INSTALL(
	TARGETS
		unbit-analyze
	
	RUNTIME 
		COMPONENT Runtime
)
