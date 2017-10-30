#
# External dependencies
#

# find_package(THIRDPARTY REQUIRED)

#
# Executable name and options
#

# Target name
set(target metacallcli)

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
	tokenizer.hpp
	tokenizer.cpp
	parser.inl
	parser.hpp
	parser.cpp
	application.hpp
	application.cpp
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
	${META_PROJECT_NAME}::version
	${META_PROJECT_NAME}::preprocessor
	${META_PROJECT_NAME}::environment
	${META_PROJECT_NAME}::format
	${META_PROJECT_NAME}::log
	${META_PROJECT_NAME}::adt
	${META_PROJECT_NAME}::reflect
	${META_PROJECT_NAME}::dynlink
	${META_PROJECT_NAME}::serial
	${META_PROJECT_NAME}::configuration
	${META_PROJECT_NAME}::loader
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

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Deployment
#

# Executable
install(TARGETS ${target}
	RUNTIME DESTINATION ${INSTALL_BIN} COMPONENT examples
	BUNDLE  DESTINATION ${INSTALL_BIN} COMPONENT examples
)

#
# Define test
#

set(TEST_COMMAND_LINE_ARGS "mock test.mock")

add_test(NAME ${target}
	COMMAND ${CMAKE_COMMAND} -E echo exit | $<TARGET_FILE:${target}> ${TEST_COMMAND_LINE_ARGS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	${TEST_COMMAND_LINE_ARGS}
	${TESTS_ENVIRONMENT_VARIABLES}
)
