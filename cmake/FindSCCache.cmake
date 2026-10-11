# Locate the sccache compiler cache and its version
find_program(SCCACHE_EXECUTABLE
	NAMES sccache
	DOC "sccache compiler cache"
)

if(SCCACHE_EXECUTABLE)
	execute_process(
		COMMAND ${SCCACHE_EXECUTABLE} --version
		RESULT_VARIABLE SCCACHE_VERSION_RESULT
		OUTPUT_VARIABLE SCCACHE_VERSION_RAW
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" SCCACHE_VERSION "${SCCACHE_VERSION_RAW}")

	if(NOT SCCACHE_VERSION_RESULT EQUAL 0 OR NOT SCCACHE_VERSION)
		message(WARNING "SCCache: cannot read the sccache version, the cache still works")
		set(SCCACHE_VERSION "")
	endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SCCache
	REQUIRED_VARS SCCACHE_EXECUTABLE
	VERSION_VAR SCCACHE_VERSION
)
