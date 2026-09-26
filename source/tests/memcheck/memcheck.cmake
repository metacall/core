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
file(GLOB old_logs "${CMAKE_BINARY_DIR}/Testing/Temporary/MemoryChecker.*.log")

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

file(GLOB logs "${CMAKE_BINARY_DIR}/Testing/Temporary/MemoryChecker.*.log")

foreach(log ${logs})
	file(READ "${log}" content)
	message("")
	message("${log}")
	message("-----------------------------------------")
	message("${content}")
endforeach()

if(NOT res EQUAL 0)
	message(FATAL_ERROR "Memcheck failed")
endif()
