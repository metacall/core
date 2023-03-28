# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_GO)
	return()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GoLang.cmake)

#
# Port name and options
#

# Target name
set(target go_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

set(CMAKE_CGO_FLAGS
	CGO_CFLAGS="-I${CMAKE_SOURCE_DIR}/source/metacall/include -Wall"
	CGO_LDFLAGS="-L${PROJECT_OUTPUT_DIR} -l$<TARGET_PROPERTY:${META_PROJECT_NAME}::metacall,NAME>$<$<CONFIG:DEBUG>:d>"
)

add_go_library(${target} SHARED
	source/await.go
	source/pointer.go
	source/go_port.go
)

#
# Define test
#

add_test(NAME ${target}
	COMMAND go test
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/source
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)
