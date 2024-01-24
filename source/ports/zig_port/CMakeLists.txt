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

# Build Zig Port
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(TARGET_BUILD_TYPE "-Doptimize=ReleaseFast")
else()
	set(TARGET_BUILD_TYPE)
endif()

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
include(Portability)

project_library_path(TEST_LIB_PATH
	${PROJECT_OUTPUT_DIR}
)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	"${PROJECT_LIBRARY_PATH_NAME}=${TEST_LIB_PATH}"
	"CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
	"PROJECT_OUTPUT_DIR=${PROJECT_OUTPUT_DIR}"
)
