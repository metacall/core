# Sanitize the variables
string(REPLACE "\"" "" GO_PPROF_DIR "${GO_PPROF_DIR}")
string(REPLACE "\"" "" GO_PPROF_BINARY "${GO_PPROF_BINARY}")
string(REPLACE "\"" "" GO_PPROF_PROFILES "${GO_PPROF_PROFILES}")

separate_arguments(GO_PPROF_PROFILES UNIX_COMMAND "${GO_PPROF_PROFILES}")

# Remove the profiles of previous runs, so they are not analyzed if the tests fail before generating them
foreach(PROFILE ${GO_PPROF_PROFILES})
	file(REMOVE "${GO_PPROF_DIR}/${PROFILE}.pprof" "${GO_PPROF_DIR}/${PROFILE}.pprof.txt")
endforeach()

# Run the tests generating the profiles, defer the error so the generated profiles are analyzed even if the tests fail
set(GO_DEFER_ERROR ON)
include("${CMAKE_CURRENT_LIST_DIR}/../source/go_port.cmake")

# Analyze the profiles
foreach(PROFILE ${GO_PPROF_PROFILES})
	set(PROFILE_FILE "${GO_PPROF_DIR}/${PROFILE}.pprof")

	set(PROFILE_SIZE 0)

	if(EXISTS "${PROFILE_FILE}")
		file(SIZE "${PROFILE_FILE}" PROFILE_SIZE)
	endif()

	if(PROFILE_SIZE EQUAL 0)
		# Profiles may not be generated or may be empty when the tests fail (i.e crash before flushing the CPU profile)
		if(GO_PORT_ERROR)
			message(WARNING "Go profile not found or empty: ${PROFILE_FILE}")
			continue()
		endif()

		message(FATAL_ERROR "Go profile not found or empty: ${PROFILE_FILE}")
	endif()

	execute_process(COMMAND
		${CMAKE_Go_COMPILER} tool pprof -text "${GO_PPROF_BINARY}" "${PROFILE_FILE}"
		RESULT_VARIABLE PPROF_RESULT
		OUTPUT_VARIABLE PPROF_OUTPUT
		ERROR_VARIABLE PPROF_ERROR
	)

	if(PPROF_RESULT)
		message(FATAL_ERROR "Go pprof failed for profile ${PROFILE} with result: ${PPROF_RESULT}\n${PPROF_ERROR}")
	endif()

	file(WRITE "${PROFILE_FILE}.txt" "${PPROF_OUTPUT}")

	message(STATUS "Go profile ${PROFILE}:\n${PPROF_OUTPUT}")
endforeach()

# Report the error of the tests after analyzing the profiles
if(GO_PORT_ERROR)
	message(FATAL_ERROR "${GO_PORT_ERROR}")
endif()
