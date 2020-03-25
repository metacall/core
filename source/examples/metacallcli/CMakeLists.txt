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

#
# Define test
#

if(WIN32)
	set(TEST_COMMAND cmd /c)
else()
	set(TEST_COMMAND sh -c)
endif()

add_test(NAME ${target}
	COMMAND ${TEST_COMMAND} "echo 'load mock a.mock\ninspect\nexit' | $<TARGET_FILE:${target}>"
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

if(OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_NODE AND OPTION_BUILD_SCRIPTS AND OPTION_BUILD_SCRIPTS_NODE)
	add_test(NAME ${target}-node
		COMMAND ${TEST_COMMAND} "echo 'load node nod.js\ninspect\ncall hello_boy(3, 4)\nexit' | $<TARGET_FILE:${target}>"
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)

	if(OPTION_BUILD_PORTS AND OPTION_BUILD_PORTS_NODE AND OPTION_BUILD_LOADERS_PY)
		add_test(NAME ${target}-node-port-py
			COMMAND $<TARGET_FILE:${target}> test.js
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)
	endif()

	if(OPTION_BUILD_PORTS AND OPTION_BUILD_PORTS_PY)
		add_test(NAME ${target}-py-port
		COMMAND ${TEST_COMMAND} "echo 'load py test.py\ninspect\ncall test()\nexit' | $<TARGET_FILE:${target}>"
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)
	endif()
endif()

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

if(OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_NODE AND OPTION_BUILD_SCRIPTS AND OPTION_BUILD_SCRIPTS_NODE)
	test_environment_variables(${target}-node
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)

	if(OPTION_BUILD_PORTS AND OPTION_BUILD_PORTS_NODE AND OPTION_BUILD_LOADERS_PY)
		set(TEST_LOADER_SCRIPT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/test")

		test_environment_variables(${target}-node-port-py
			""
			"LOADER_LIBRARY_PATH=${LOADER_LIBRARY_PATH}"
			"LOADER_SCRIPT_PATH=${TEST_LOADER_SCRIPT_PATH}"
			"CONFIGURATION_PATH=${CONFIGURATION_PATH}"
			"SERIAL_LIBRARY_PATH=${SERIAL_LIBRARY_PATH}"
			"DETOUR_LIBRARY_PATH=${DETOUR_LIBRARY_PATH}"
			"PORT_LIBRARY_PATH=${PORT_LIBRARY_PATH}"
		)
	endif()
endif()
