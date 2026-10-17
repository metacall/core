include(ProcessorCount)

ProcessorCount(N)

# Run only the tests matching a regex, for example:
#	MEMCHECK_TEST=adt-vector-test make memcheck
if(NOT DEFINED MEMCHECK_TEST AND DEFINED ENV{MEMCHECK_TEST})
	set(MEMCHECK_TEST "$ENV{MEMCHECK_TEST}")
endif()

if(MEMCHECK_TEST)
	set(MEMCHECK_TEST_FILTER --tests-regex "${MEMCHECK_TEST}")
endif()

# Remove the logs of previous runs so only the tests run now are printed
file(GLOB old_logs
	"${CMAKE_BINARY_DIR}/Testing/Temporary/MemoryChecker.*.log"
	"${CMAKE_BINARY_DIR}/Testing/Temporary/LastTestsFailed*.log"
)

if(old_logs)
	file(REMOVE ${old_logs})
endif()

execute_process(
	COMMAND ${CMAKE_CTEST_COMMAND}
		-j${N}
		--label-exclude MEMCHECK_IGNORE
		${MEMCHECK_TEST_FILTER}
		--force-new-ctest-process
		--test-action memcheck
		--timeout 5400
	RESULT_VARIABLE res
)

if(res EQUAL 0)
	return()
endif()

# Print only the logs of the failed tests, the output of the CI gets truncated
# when it is too big. The list has one "<number>:<name>" line per failed test,
# and ctest names it LastTestsFailed_<tag>.log when using --test-action
file(GLOB failed_lists "${CMAKE_BINARY_DIR}/Testing/Temporary/LastTestsFailed*.log")

set(failed_tests)

foreach(failed_list ${failed_lists})
	file(STRINGS "${failed_list}" lines)
	list(APPEND failed_tests ${lines})
endforeach()

if(NOT failed_tests)
	# Do not hide anything if ctest failed without listing the tests
	file(GLOB logs "${CMAKE_BINARY_DIR}/Testing/Temporary/MemoryChecker.*.log")

	foreach(log ${logs})
		file(READ "${log}" content)
		message("")
		message("${log}")
		message("-----------------------------------------")
		message("${content}")
	endforeach()

	message(FATAL_ERROR "Memcheck failed")
endif()

foreach(failed_test ${failed_tests})
	string(REGEX MATCH "^([0-9]+):(.*)$" match "${failed_test}")
	set(number "${CMAKE_MATCH_1}")
	set(name "${CMAKE_MATCH_2}")
	set(log "${CMAKE_BINARY_DIR}/Testing/Temporary/MemoryChecker.${number}.log")

	message("")
	message("${name}")
	message("-----------------------------------------")

	if(EXISTS "${log}")
		file(READ "${log}" content)
		message("${content}")
	else()
		message("No memcheck log found (${log})")
	endif()
endforeach()

message("")
message("Memcheck failed tests:")

foreach(failed_test ${failed_tests})
	string(REGEX REPLACE "^([0-9]+):" "\\1 - " name "${failed_test}")
	message("	${name}")
endforeach()

message(FATAL_ERROR "Memcheck failed")
