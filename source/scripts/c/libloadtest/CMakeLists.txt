#
# Configure C project
#

c_project(libloadtest 0.1.0)

# Build the library
add_library(loadtest SHARED
	source/libloadtest.cpp
	source/libloadtest.h
)

# Configure library path
set(PROJECT_LIBRARY_PATH "${PROJECT_OUTPUT_DIR}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/source/libloadtest.ld.in" "${LOADER_SCRIPT_PATH}/libloadtest.ld" @ONLY)
