# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY)
	return()
endif()

#
# External dependencies
#

return() # TODO:

# MetaCall Ports
find_package(MetaCallPorts REQUIRED)

if(NOT METACALL_PORTS_FOUND OR NOT METACALL_PORTS_PY_FOUND)
	message(STATUS "MetaCall ports not found")
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacallquine)

# Exit here if required dependencies are not met
message(STATUS "Example ${target}")

#
# Compiler warnings
#

include(Warnings)

#
# Compiler security
#

include(SecurityFlags)

#
# Sources
#

set(sources
	main.cpp
)

#
# Create executable
#

# Build executable
add_executable(${target}
	MACOSX_BUNDLE
	${sources}
)

# Create namespaced alias
add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

#
# Project options
#

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

#
# Include directories
#

target_include_directories(${target}
	PRIVATE
	${DEFAULT_INCLUDE_DIRECTORIES}
	${PROJECT_BINARY_DIR}/source/include
)

#
# Libraries
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LIBRARIES}
	${META_PROJECT_NAME}::metacall
)

#
# Compile definitions
#

target_compile_definitions(${target}
	PRIVATE
	${DEFAULT_COMPILE_DEFINITIONS}
)

#
# Compile options
#

target_compile_options(${target}
	PRIVATE
	${DEFAULT_COMPILE_OPTIONS}
)

#
# Linker options
#

add_link_options(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)
