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
message(STATUS "CLI ${target}")

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

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")
set(inline_path "${CMAKE_CURRENT_SOURCE_DIR}/inline/${target}")
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(headers
	${include_path}/tokenizer.hpp
	${include_path}/parser.hpp
	${include_path}/application.hpp
)

set(inline
	${inline_path}/parser.inl
)

set(sources
	${source_path}/tokenizer.cpp
	${source_path}/parser.cpp
	${source_path}/application.cpp
	${source_path}/main.cpp
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
# Target for scripts (testing)
#

set(PROJECT_METACALL_PORTS_DIRECTORY "${CMAKE_SOURCE_DIR}/source/ports/py_port")

add_custom_target(${target}-scripts-tests ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} -E  make_directory ${LOADER_SCRIPT_PATH}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test/ ${LOADER_SCRIPT_PATH}/
	COMMAND ${CMAKE_COMMAND} -DPROJECT_METACALL_PORTS_DIRECTORY=${PROJECT_METACALL_PORTS_DIRECTORY} -DLOADER_SCRIPT_PATH=${LOADER_SCRIPT_PATH} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure_tests.cmake
)

set_target_properties(${target}-scripts-tests
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

add_dependencies(${target} ${target}-scripts-tests)

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
	${PROJECT_BINARY_DIR}/source/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include
	${PROJECT_BINARY_DIR}/source/inline
	${CMAKE_CURRENT_SOURCE_DIR}/inline
	${CMAKE_CURRENT_BINARY_DIR}/inline
	${DEFAULT_INCLUDE_DIRECTORIES}

	PUBLIC

	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<INSTALL_INTERFACE:include>
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
	RUNTIME DESTINATION ${INSTALL_BIN} COMPONENT cli
	BUNDLE  DESTINATION ${INSTALL_BIN} COMPONENT cli
)

#
# Define test
#

if(WIN32)
	set(TEST_COMMAND cmd /c)
	set(GREP_COMMAND findstr)
else()
	set(TEST_COMMAND sh -c)
	set(GREP_COMMAND grep)
endif()

add_test(NAME ${target}
	COMMAND ${TEST_COMMAND} "echo 'load mock a.mock\ninspect\nexit' | $<TARGET_FILE:${target}> | ${GREP_COMMAND} \"function three_str(a_str, b_str, c_str)\""
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Define dependencies
#

include(GeneratorExpressions)

cmake_expand_generator_expressions(DEPENDENCIES
	$<$<BOOL:${OPTION_BUILD_LOADERS_NODE}>:node_loader>
	$<$<BOOL:${OPTION_BUILD_LOADERS_PY}>:py_loader>
	$<$<BOOL:${OPTION_BUILD_LOADERS_RB}>:rb_loader>
	$<$<BOOL:${OPTION_BUILD_LOADERS_CS}>:cs_loader>
	$<$<BOOL:${OPTION_BUILD_LOADERS_JSM}>:jsm_loader>
	$<$<BOOL:${OPTION_BUILD_LOADERS_JS}>:js_loader>
	$<$<BOOL:${OPTION_BUILD_LOADERS_MOCK}>:mock_loader>
	$<$<BOOL:${OPTION_BUILD_LOADERS_C}>:c_loader>
	$<$<BOOL:${OPTION_BUILD_LOADERS_FILE}>:file_loader>
)

if(DEPENDENCIES)
	add_dependencies(${target} ${DEPENDENCIES})
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
	add_test(NAME ${target}-node
		COMMAND ${TEST_COMMAND} "echo 'load node nod.js\ninspect\ncall hello_boy(300, 400)\nexit' | $<TARGET_FILE:${target}> | ${GREP_COMMAND} \"700.0\""
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	test_environment_variables(${target}-node
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)

	if(OPTION_BUILD_PORTS AND OPTION_BUILD_PORTS_NODE AND OPTION_BUILD_LOADERS_PY)
		add_test(NAME ${target}-node-port-py
		COMMAND ${TEST_COMMAND} "echo 'load node cli-test.js\ninspect\nexit' | $<TARGET_FILE:${target}> | ${GREP_COMMAND} \"ABCDEFGHIJKLMNOPQRSTUVWXYZ\""
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)
		test_environment_variables(${target}-node-port-py
			""
			${TESTS_ENVIRONMENT_VARIABLES}
		)

		if(OPTION_BUILD_LOADERS_RB)
			add_test(NAME ${target}-node-port-py-rb
			COMMAND ${TEST_COMMAND} "echo 'load node cli-test-onload.js\ninspect\nexit' | $<TARGET_FILE:${target}>"
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			)
			test_environment_variables(${target}-node-port-py-rb
				""
				${TESTS_ENVIRONMENT_VARIABLES}
			)
		endif()
	endif()

	add_test(NAME ${target}-node-null
	COMMAND ${TEST_COMMAND} "echo 'load node cli-test-null-undefined.js\ninspect\ncall test_null_and_undefined(null)\nexit' | $<TARGET_FILE:${target}> | ${GREP_COMMAND} \"Hello 342521512461246!\""
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	test_environment_variables(${target}-node-null
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)

	add_test(NAME ${target}-node-null-empty
	COMMAND ${TEST_COMMAND} "echo 'load node cli-test-null-undefined.js\ninspect\ncall test_null_and_undefined()\nexit' | $<TARGET_FILE:${target}> | ${GREP_COMMAND} \"Hello 342521512461246!\""
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	test_environment_variables(${target}-node-null-empty
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)

	add_test(NAME ${target}-node-null-undefined
	COMMAND ${TEST_COMMAND} "echo 'load node cli-test-null-undefined.js\ninspect\ncall test_null_and_undefined(undefined)\nexit' | $<TARGET_FILE:${target}> | ${GREP_COMMAND} \"(null)\""
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	test_environment_variables(${target}-node-null-undefined
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)

	if(OPTION_BUILD_PORTS AND OPTION_BUILD_PORTS_PY)
		add_test(NAME ${target}-py-port
			COMMAND ${TEST_COMMAND} "echo 'load py cli-test.py\ninspect\ncall test()\nexit' | $<TARGET_FILE:${target}> | ${GREP_COMMAND} \"1234\""
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)
		test_environment_variables(${target}-py-port
			""
			${TESTS_ENVIRONMENT_VARIABLES}
		)
	endif()

	if(OPTION_BUILD_PORTS AND OPTION_BUILD_PORTS_PY AND OPTION_BUILD_LOADERS_RB)
		add_test(NAME ${target}-py-port-rb
			COMMAND ${TEST_COMMAND} "echo 'load py cli-test-rb.py\ninspect\ncall test()\nexit' | $<TARGET_FILE:${target}> | ${GREP_COMMAND} \"0123456789ABCDEFasd\""
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)
		test_environment_variables(${target}-py-port-rb
			""
			${TESTS_ENVIRONMENT_VARIABLES}
		)
	endif()
endif()

if(OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_FILE AND OPTION_BUILD_SCRIPTS AND OPTION_BUILD_SCRIPTS_FILE)
	add_test(NAME ${target}-file
		COMMAND ${TEST_COMMAND} "echo 'load file template.html\ninspect\ncall template.html()\nexit' | $<TARGET_FILE:${target}> | ${GREP_COMMAND} \"${LOADER_SCRIPT_PATH}/template.html\""
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	test_environment_variables(${target}-file
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)
endif()

if(OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_TS AND OPTION_BUILD_SCRIPTS AND OPTION_BUILD_SCRIPTS_TS)
	add_test(NAME ${target}-ts
		COMMAND ${TEST_COMMAND} "echo 'load ts typedfunc.ts\ninspect\ncall typed_sum(4, 5)\nexit' | $<TARGET_FILE:${target}> | ${GREP_COMMAND} \"9.0\""
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	test_environment_variables(${target}-ts
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)
endif()
