include(ProcessorCount)

ProcessorCount(N)

execute_process(
	COMMAND ${CMAKE_CTEST_COMMAND}
		-j${N}
		--label-exclude MEMCHECK_IGNORE
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
