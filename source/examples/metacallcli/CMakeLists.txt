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

# Configure package manager path
if(MSVC)
	set(PROJECT_METACALL_PACK_MAN_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
	set(PROJECT_METACALL_PACK_MAN_DIR "${CMAKE_BINARY_DIR}")
endif()

# Copy package manager scripts
add_custom_target(${target}-scripts ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package_manager/pip.py ${PROJECT_METACALL_PACK_MAN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package_manager/npm.js ${PROJECT_METACALL_PACK_MAN_DIR}
)

# Set dependency to metacallcli
add_dependencies(${target} ${target}-scripts)

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
	${META_PROJECT_NAME}::metacall_distributable
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

# Scripts (Package Managers)
install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/package_manager/pip.py
	${CMAKE_CURRENT_SOURCE_DIR}/package_manager/npm.js
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)

#
# Define test
#

#set(TEST_COMMAND_LINE_ARGS "help") # TODO: Forward all commands, create an adaptor to existing system
set(TEST_COMMAND_LINE_ARGS "load mock test.mock")
#set(TEST_COMMAND_LINE_ARGS "install py numpy")
#set(TEST_COMMAND_LINE_ARGS "install node rambda") # TODO: This segfaults during ctest but not when running it normally

if(WIN32)
	set(TEST_COMMAND cmd /c)
else()
	set(TEST_COMMAND sh -c)
endif()

add_test(NAME ${target}
	COMMAND ${TEST_COMMAND} "echo 'inspect\nexit' | $<TARGET_FILE:${target}> ${TEST_COMMAND_LINE_ARGS}"
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)

if(NOT OPTION_BUILD_GUIX)
	# Get node path for npm
	execute_process(
		COMMAND npm root --quiet -g
		OUTPUT_VARIABLE NODE_PATH
	)
	set(NODE_PATH "NODE_PATH=${NODE_PATH}")
else()
	set(NODE_PATH "NODE_PATH=$ENV{NODE_PATH}")
endif()

test_environment_variables(${target}
	${TEST_COMMAND_LINE_ARGS}
	${TESTS_ENVIRONMENT_VARIABLES}
	${NODE_PATH}
)
