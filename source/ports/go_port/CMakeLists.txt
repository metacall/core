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
	CGO_CFLAGS='-I${CMAKE_SOURCE_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/include'
	CGO_LDFLAGS='-L${PROJECT_OUTPUT_DIR} -l$<TARGET_PROPERTY:${META_PROJECT_NAME}::metacall,NAME>$<$<CONFIG:DEBUG>:d>'
)

add_go_library(${target} SHARED
	source/await.go
	source/pointer.go
	source/go_port.go
)

#
# Define test
#

include(Portability)

add_test(NAME ${target}
	COMMAND ${CMAKE_COMMAND}
		-DGOPATH="${GOPATH}"
		-DCMAKE_Go_COMPILER="${CMAKE_Go_COMPILER}"
		-DCGO_CFLAGS="-I${CMAKE_SOURCE_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/include -Wall"
		-DCGO_LDFLAGS="-L${PROJECT_OUTPUT_DIR} -l$<TARGET_PROPERTY:${META_PROJECT_NAME}::metacall,NAME>$<$<CONFIG:DEBUG>:d>"
		-DLIBRARY_PATH_NAME="${PROJECT_LIBRARY_PATH_NAME}"
		-DLIBRARY_PATH="${PROJECT_OUTPUT_DIR}"
		-P ${CMAKE_CURRENT_SOURCE_DIR}/source/test.cmake
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)
