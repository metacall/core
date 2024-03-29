# Check if this port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_ZIG)
	return()
endif()

#
# Port name and options
#

find_package(Zig)

if(NOT Zig_FOUND)
	message(SEND_ERROR "Zig not found")
	return()
endif()

# Target name
set(target zig_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

# Generate bindings
add_custom_target(TARGET ${target}
	COMMAND ${Zig_COMPILER_EXECUTABLE} translate-c metacall.h > metacall-bindings.zig -lc -I ${CMAKE_SOURCE_DIR}/source/metacall/include -I ${CMAKE_BINARY_DIR}/source/metacall/include -I ${CMAKE_BINARY_DIR}/source/include
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#
# Define test
#

add_test(NAME ${target}
	COMMAND ${Zig_COMPILER_EXECUTABLE} build test
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)
