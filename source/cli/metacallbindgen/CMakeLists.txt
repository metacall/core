#
# Executable name and options
#

# Target name
set(target metacallbindgen)

# Exit here if required dependencies are not met
find_package(LibClang)

if(NOT LibClang_FOUND)
	message(STATUS "CLI ${target} skipped (LibClang not found)")
	return()
endif()

message(STATUS "CLI ${target}")

#
# Compiler warnings / security
#

include(Warnings)
include(SecurityFlags)

#
# Sources
#

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(sources
	${source_path}/main.cpp
)

#
# Create executable
#

add_executable(${target}
	${sources}
)

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
	${LibClang_INCLUDE_DIR}
)

#
# Libraries
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LIBRARIES}
	${LibClang_LIBRARY}
)

#
# Compile definitions / options / features / linker options
#

target_compile_definitions(${target}
	PRIVATE
	${DEFAULT_COMPILE_DEFINITIONS}
)

target_compile_options(${target}
	PRIVATE
	${DEFAULT_COMPILE_OPTIONS}
)

target_compile_features(${target}
	PRIVATE
	cxx_std_17
)

target_link_options(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)
