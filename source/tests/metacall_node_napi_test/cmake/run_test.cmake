if(NOT DEFINED EXECUTABLE)
	message(FATAL_ERROR "EXECUTABLE was not provided")
endif()

if(NOT DEFINED TEST_DIR)
	message(FATAL_ERROR "TEST_DIR was not provided")
endif()

if(NOT DEFINED NPM_EXECUTABLE)
	message(FATAL_ERROR "NPM_EXECUTABLE was not provided")
endif()

if(NOT EXISTS "${EXECUTABLE}")
	message(FATAL_ERROR "MetaCall CLI executable not found: ${EXECUTABLE}")
endif()

set(rspack_package "${TEST_DIR}/node_modules/@rspack/core/package.json")

if(NOT EXISTS "${rspack_package}")
	message(STATUS "Installing @rspack/core into ${TEST_DIR}")

	execute_process(
		COMMAND "${NPM_EXECUTABLE}" --prefix "${TEST_DIR}" install @rspack/core
		WORKING_DIRECTORY "${TEST_DIR}"
		RESULT_VARIABLE npm_result
		OUTPUT_VARIABLE npm_output
		ERROR_VARIABLE npm_error
	)

	if(NOT "${npm_output}" STREQUAL "")
		message("${npm_output}")
	endif()

	if(NOT "${npm_error}" STREQUAL "")
		message("${npm_error}")
	endif()

	if(NOT npm_result EQUAL 0)
		message(FATAL_ERROR "Failed to install @rspack/core (exit code ${npm_result})")
	endif()
endif()

message(STATUS "Running MetaCall Node NAPI rspack regression test")

execute_process(
	COMMAND "${EXECUTABLE}"
	INPUT_FILE "${TEST_DIR}/commands.txt"
	WORKING_DIRECTORY "${TEST_DIR}"
	RESULT_VARIABLE cli_result
	OUTPUT_VARIABLE cli_output
	ERROR_VARIABLE cli_error
)

if(NOT "${cli_output}" STREQUAL "")
	message("${cli_output}")
endif()

if(NOT "${cli_error}" STREQUAL "")
	message("${cli_error}")
endif()

if(NOT cli_result EQUAL 0)
	message(FATAL_ERROR "MetaCall CLI regression test failed (exit code ${cli_result})")
endif()
