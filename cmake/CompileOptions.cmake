#
# Compile options configuration
#

option(OPTION_BUILD_ADDRESS_SANITIZER	"Build with sanitizer compiler options."						OFF)
option(OPTION_BUILD_THREAD_SANITIZER	"Build with thread sanitizer compiler options."					OFF)
option(OPTION_BUILD_MEMORY_SANITIZER	"Build with memory sanitizer compiler options."					OFF)

if((OPTION_BUILD_ADDRESS_SANITIZER AND OPTION_BUILD_MEMORY_SANITIZER) OR (OPTION_BUILD_ADDRESS_SANITIZER AND OPTION_BUILD_THREAD_SANITIZER) OR (OPTION_BUILD_MEMORY_SANITIZER AND OPTION_BUILD_THREAD_SANITIZER))
	message(FATAL_ERROR "OPTION_BUILD_ADDRESS_SANITIZER and OPTION_BUILD_MEMORY_SANITIZER and OPTION_BUILD_THREAD_SANITIZER are mutually exclusive, choose one of them")
endif()

#
# Platform and architecture setup
#

# Get upper case system name
string(TOUPPER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_UPPER)

# Determine architecture (32/64 bit)
set(X64 OFF)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(X64 ON)
endif()

#
# Include portability
#

include(Portability)

#
# Project options
#

# Test for GNU 4.9+, Clang 3.6+ or ((Visual Studio C++ or Clang with MSVC backend) and Visual Studio 2022 or superior)
if(("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" AND ${CMAKE_C_COMPILER_VERSION} VERSION_GREATER 4.9) OR
	((("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang") OR
	("${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")) AND ${CMAKE_C_COMPILER_VERSION} VERSION_GREATER 3.6) OR
	(
		(("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC") OR
		(("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang") AND ("${CMAKE_C_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC"))) AND
		(MSVC_VERSION GREATER_EQUAL 1930)
	)
)
	set(C_STANDARD 11)
else()
	set(C_STANDARD 99) # TODO: Implement support for older standards
endif()

set(DEFAULT_PROJECT_OPTIONS
	DEBUG_POSTFIX				"d"
	CXX_STANDARD				11
	LINKER_LANGUAGE				"CXX"
	POSITION_INDEPENDENT_CODE	ON
	CXX_VISIBILITY_PRESET		"hidden"
	C_STANDARD					${C_STANDARD}
)

#
# Include directories
#

set(DEFAULT_INCLUDE_DIRECTORIES)

#
# Libraries
#

# Valgrind
if(OPTION_TEST_MEMORYCHECK)
	set(MEMORYCHECK_COMPILE_DEFINITIONS
		"__MEMORYCHECK__=1"
	)

	set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full")
	# set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --show-leak-kinds=all")
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --trace-children=yes")
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --show-reachable=yes")
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --track-origins=yes")
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --num-callers=100")
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --smc-check=all-non-file") # for JITs
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --suppressions=${CMAKE_SOURCE_DIR}/source/tests/memcheck/valgrind-dl.supp")
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --suppressions=${CMAKE_SOURCE_DIR}/source/tests/memcheck/valgrind-python.supp")
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --suppressions=${CMAKE_SOURCE_DIR}/source/tests/memcheck/valgrind-node.supp")
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --suppressions=${CMAKE_SOURCE_DIR}/source/tests/memcheck/valgrind-wasm.supp")
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --suppressions=${CMAKE_SOURCE_DIR}/source/tests/memcheck/valgrind-wasm.supp")

	# TODO: Implement automatic detection for valgrind suppressions and create a proper test suite for the CI
	set(MEMORYCHECK_ADDITIONAL_SUPPRESSIONS
		"/usr/lib/valgrind/python3.supp"
		"/usr/lib/valgrind/debian.supp"
	)

	foreach(SUPPRESSION ${MEMORYCHECK_ADDITIONAL_SUPPRESSIONS})
		if(EXISTS "${SUPPRESSION}")
			set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --suppressions=${SUPPRESSION}")
		endif()
	endforeach()

	# This is needed in order to allow valgrind to properly track malloc in Python
	set(TESTS_MEMCHECK_ENVIRONMENT_VARIABLES
		"PYTHONMALLOC=malloc"
	)
else()
	set(MEMORYCHECK_COMPILE_DEFINITIONS)
	set(MEMORYCHECK_COMMAND_OPTIONS)
	set(TESTS_MEMCHECK_ENVIRONMENT_VARIABLES)
endif()

# ThreadSanitizer is incompatible with AddressSanitizer and LeakSanitizer
if(OPTION_BUILD_THREAD_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
	set(SANITIZER_LIBRARIES -ltsan)
	set(TESTS_SANITIZER_ENVIRONMENT_VARIABLES
		"TSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/source/tests/sanitizer/tsan.supp"
	)
	set(SANITIZER_COMPILE_DEFINITIONS
		"__THREAD_SANITIZER__=1"
	)
elseif(OPTION_BUILD_MEMORY_SANITIZER AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
	# TODO: This requires much more effort than expected: https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo
	set(SANITIZER_LIBRARIES)
	set(TESTS_SANITIZER_ENVIRONMENT_VARIABLES)
	set(SANITIZER_COMPILE_DEFINITIONS
		"__MEMORY_SANITIZER__=1"
	)
elseif(OPTION_BUILD_ADDRESS_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
	set(SANITIZER_LIBRARIES -lasan -lubsan)
	set(TESTS_SANITIZER_ENVIRONMENT_VARIABLES
		"LSAN_OPTIONS=verbosity=1:log_threads=1:print_suppressions=false:suppressions=${CMAKE_SOURCE_DIR}/source/tests/sanitizer/lsan.supp"

		# Specify handle_segv=0 and detect_leaks=0 for the JVM (https://blog.gypsyengineer.com/en/security/running-java-with-addresssanitizer.html)
		# "ASAN_OPTIONS=handle_segv=0:symbolize=1:alloc_dealloc_mismatch=0:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:fast_unwind_on_malloc=0"

		# TODO: We should document each flag why is it used, because now we do not know what runtime has each requirement and why.
		# Another option should be to separate by runtimes and only set up them on the ASAN tests that require them,
		# because we do not need to disable all features on all tests, this may hide bugs in the core library for example.

		# Specify use_sigaltstack=0 as CoreCLR uses own alternate stack for signal handlers (https://github.com/swgillespie/coreclr/commit/bec020aa466d08e49e007d0011b0e79f8f7c7a62)
		"ASAN_OPTIONS=use_sigaltstack=0:symbolize=1:alloc_dealloc_mismatch=0:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:fast_unwind_on_malloc=0"
	)
	set(SANITIZER_COMPILE_DEFINITIONS
		"__ADDRESS_SANITIZER__=1"
	)
else()
	set(SANITIZER_LIBRARIES)
	set(TESTS_SANITIZER_ENVIRONMENT_VARIABLES)
	set(SANITIZER_COMPILE_DEFINITIONS)
endif()

function(find_sanitizer NAME LINK_OPTION)
	string(TOUPPER "${NAME}" NAME_UPPER)
	set(SANITIZER_PROGRAM_CODE "int main() {return 0;}")
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/sanitizer_locate.cpp" "${SANITIZER_PROGRAM_CODE}")

	try_compile(
		STATUS
			${PROJECT_OUTPUT_DIR}
			${CMAKE_CURRENT_BINARY_DIR}/sanitizer_locate.cpp
		OUTPUT_VARIABLE SANITIZER_COMPILER_OUTPUT
		LINK_OPTIONS ${LINK_OPTION}
		COPY_FILE ${CMAKE_CURRENT_BINARY_DIR}/sanitizer_locate
	)

	if(NOT STATUS)
		message(FATAL_ERROR "Could not find location for lib${NAME}: ${SANITIZER_COMPILER_OUTPUT}")
		return()
	endif()

	file(GET_RUNTIME_DEPENDENCIES
		EXECUTABLES ${CMAKE_CURRENT_BINARY_DIR}/sanitizer_locate
		RESOLVED_DEPENDENCIES_VAR SANITIZER_PROGRAM_LIBRARIES
	)

	foreach(DEPENDENCY IN LISTS SANITIZER_PROGRAM_LIBRARIES)
		string(FIND "${DEPENDENCY}" "${NAME}" POSITION)
		if(POSITION GREATER -1)
			set(LIB${NAME_UPPER}_PATH "${DEPENDENCY}" PARENT_SCOPE)
			return()
		endif()
	endforeach()
endfunction()

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
	if(OPTION_BUILD_THREAD_SANITIZER)
		find_sanitizer(tsan -fsanitize=thread)
		set(SANITIZER_LIBRARIES_PATH
			"${LIBTSAN_PATH}"
		)
	elseif(OPTION_BUILD_MEMORY_SANITIZER)
		set(SANITIZER_LIBRARIES_PATH) # TODO
	elseif(OPTION_BUILD_ADDRESS_SANITIZER)
		find_sanitizer(asan -fsanitize=address)
		find_sanitizer(ubsan -fsanitize=undefined)
		set(SANITIZER_LIBRARIES_PATH
			"${LIBASAN_PATH}"
			"${LIBUBSAN_PATH}"
		)
	endif()
endif()

if(SANITIZER_LIBRARIES_PATH)
	if(PROJECT_OS_LINUX)
		list(JOIN SANITIZER_LIBRARIES_PATH " " SANITIZER_LIBRARIES_PATH_JOINED)
		set(TESTS_SANITIZER_PRELOAD_ENVIRONMENT_VARIABLES
			"LD_PRELOAD=${SANITIZER_LIBRARIES_PATH_JOINED}"
		)
	elseif(PROJECT_OS_FAMILY MATCHES "macos")
		list(JOIN SANITIZER_LIBRARIES_PATH ":" SANITIZER_LIBRARIES_PATH_JOINED)
		set(TESTS_SANITIZER_PRELOAD_ENVIRONMENT_VARIABLES
			"DYLD_INSERT_LIBRARIES=${SANITIZER_LIBRARIES_PATH_JOINED}"
			"DYLD_FORCE_FLAT_NAMESPACE=1"
		)
	endif()
endif()

if((PROJECT_OS_WIN AND MSVC) OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	# MSVC and Clang do not require to link manually the sanitizer libraries
	set(SANITIZER_LIBRARIES)
endif()

# Set default libraries
set(DEFAULT_LIBRARIES
	${SANITIZER_LIBRARIES}
)

#
# Compile definitions
#

if(OPTION_BUILD_LOG_PRETTY)
	set(LOG_POLICY_FORMAT_PRETTY_VALUE 1)
else()
	set(LOG_POLICY_FORMAT_PRETTY_VALUE 0)
endif()

if(OPTION_MEMORY_TRACKER)
	set(REFLECT_MEMORY_TRACKER_VALUE 1)
else()
	set(REFLECT_MEMORY_TRACKER_VALUE 0)
endif()

set(DEFAULT_COMPILE_DEFINITIONS
	LOG_POLICY_FORMAT_PRETTY=${LOG_POLICY_FORMAT_PRETTY_VALUE}
	REFLECT_MEMORY_TRACKER=${REFLECT_MEMORY_TRACKER_VALUE}
	SYSTEM_${SYSTEM_NAME_UPPER}
	${MEMORYCHECK_COMPILE_DEFINITIONS}
	${SANITIZER_COMPILE_DEFINITIONS}
)

# MSVC compiler options
if(PROJECT_OS_WIN AND MSVC)
	set(DEFAULT_COMPILE_DEFINITIONS ${DEFAULT_COMPILE_DEFINITIONS}
		_SCL_SECURE_NO_WARNINGS	# Calling any one of the potentially unsafe methods in the Standard C++ Library
		_CRT_SECURE_NO_WARNINGS	# Calling any one of the potentially unsafe methods in the CRT Library
	)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(DEFAULT_COMPILE_DEFINITIONS
		${DEFAULT_COMPILE_DEFINITIONS}
		DEBUG
	)
else()
	set(DEFAULT_COMPILE_DEFINITIONS
		${DEFAULT_COMPILE_DEFINITIONS}
		NDEBUG
	)
endif()

#
# Compile options
#

# They are empty by default
set(DEFAULT_COMPILE_OPTIONS)

if(WIN32 AND MSVC)
	# Build runtime as multithreaded shared library
	# if(${CMAKE_VERSION} VERSION_LESS "3.15")
	# 	set(COMPILER_FLAGS_ID
	# 		CMAKE_CXX_FLAGS
	# 		CMAKE_CXX_FLAGS_DEBUG
	# 		CMAKE_CXX_FLAGS_RELEASE
	# 		CMAKE_C_FLAGS
	# 		CMAKE_C_FLAGS_DEBUG
	# 		CMAKE_C_FLAGS_RELEASE
	# 	)
	# 	foreach(FLAG_ID ${COMPILER_FLAGS_ID})
	# 		string(REPLACE "/MD" "/MT" ${FLAG_ID} "${${FLAG_ID}}")
	# 	endforeach()
	# else()
	# 	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
	# endif()

	add_compile_options(/nologo) # Suppress Startup Banner
	add_compile_options(/Gm-) # Disable minimal rebuild
	add_compile_options(/MP) # Build with Multiple Processes (number of processes equal to the number of processors)
	#add_compile_options(/wd4251 /wd4592)
	#add_compile_options(/ZH:SHA_256) # use SHA256 for generating hashes of compiler processed source files.

	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		# Disable optimizations
		add_compile_options(/Od)
	else()
		# Enable read-only string pooling
		add_compile_options(/GF)

		# Buffer Security Check
		add_compile_options(/GS)

		# Disable Run-Time Error Checks
		add_compile_options(/GR-)

		# Enable optimizations
		# add_compile_options(/O2) # TODO: Enable when runtime checks can be disabled properly
		add_compile_options(/Oi)
		add_compile_options(/Oy)

		# TODO: Disable runtime checks (not compatible with O2)
		# foreach(COMPILER_FLAGS
		# 	CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE
		# 	CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
		# 	CMAKE_C_FLAGS CMAKE_C_FLAGS_RELEASE
		# 	CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
		# 	)
		# 	string(REGEX REPLACE "/RTC[^ ]*" "" ${COMPILER_FLAGS} "${${COMPILER_FLAGS}}")
		# endforeach(COMPILER_FLAGS)

		if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
			# Enable debug symbols
			add_compile_options(/Z7)
		endif()
	endif()

	# Sanitizers
	if(OPTION_BUILD_THREAD_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		add_compile_options(/fsanitize=thread)
		add_link_options(/INCREMENTAL:NO)
	elseif(OPTION_BUILD_ADDRESS_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		add_compile_options(/fsanitize=address)
		# add_compile_options(/fsanitize=undefined)
		add_link_options(/INCREMENTAL:NO)
	elseif(OPTION_BUILD_MEMORY_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		add_compile_options(/fsanitize=memory)
		add_compile_options(/fsanitize=leak)
		add_link_options(/INCREMENTAL:NO)
	endif()

endif()

if (PROJECT_OS_FAMILY MATCHES "unix" OR PROJECT_OS_FAMILY MATCHES "macos")

	if(PROJECT_OS_FAMILY MATCHES "macos")
		# We cannot enable "stack-protector-strong" On OS X due to a bug in clang compiler (current version 7.0.2)

		# Enable threads in OS X
		add_compile_options(-pthread)

		# clang options only
		add_compile_options(-Wreturn-stack-address)
	endif()

	if(PROJECT_OS_HAIKU)
		add_compile_options(-fPIC)
	endif()

	# All warnings that are not explicitly disabled are reported as errors
	#add_compile_options(-Werror)
	add_compile_options(-Wall)
	add_compile_options(-Wextra)

	# Sanitizers
	if(OPTION_BUILD_THREAD_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		add_compile_options(-fno-omit-frame-pointer)
		add_compile_options(-fno-optimize-sibling-calls)
		add_compile_options(-fsanitize=thread)
		if(PROJECT_OS_FAMILY MATCHES "macos" OR (PROJECT_OS_FAMILY MATCHES "unix" AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
			add_link_options(-fsanitize=thread)
		endif()
	elseif(OPTION_BUILD_ADDRESS_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		add_compile_options(-fno-omit-frame-pointer)
		add_compile_options(-fno-optimize-sibling-calls)
		add_compile_options(-fsanitize=undefined)
		add_compile_options(-fsanitize=address)
		add_compile_options(-fsanitize-address-use-after-scope)
		add_compile_options(-fsanitize=float-divide-by-zero)
		add_compile_options(-fsanitize=float-cast-overflow)
		if(PROJECT_OS_FAMILY MATCHES "unix")
			if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
				add_compile_options(-fsanitize=pointer-compare)
				add_compile_options(-fsanitize=pointer-subtract)
				add_compile_options(-fuse-ld=gold)
			endif()
			add_compile_options(-fsanitize=leak)
		endif()
		if(PROJECT_OS_FAMILY MATCHES "macos" OR (PROJECT_OS_FAMILY MATCHES "unix" AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
			add_link_options(-fsanitize=undefined)
			add_link_options(-fsanitize=address)
			add_link_options(-fsanitize-address-use-after-scope)
		endif()
	elseif(OPTION_BUILD_MEMORY_SANITIZER AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		add_compile_options(-fno-omit-frame-pointer)
		add_compile_options(-fno-optimize-sibling-calls)
		add_compile_options(-fsanitize=undefined)
		add_compile_options(-fsanitize=memory)
		add_compile_options(-fsanitize-memory-track-origins)
		add_compile_options(-fsanitize-memory-use-after-dtor)
		if(PROJECT_OS_FAMILY MATCHES "macos")
			add_link_options(-fsanitize=undefined)
			add_link_options(-fsanitize=memory)
			add_link_options(-fsanitize-memory-track-origins)
			add_link_options(-fsanitize-memory-use-after-dtor)
		endif()
	endif()

	# Debug symbols
	if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		add_compile_options(-g)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
	endif()

	# Optimizations
	if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		add_compile_options(-O3)
	endif()
endif()

macro(check_symbol_executable symbol binary_path result_var)
	if(WIN32)
		find_program(DUMPBIN_EXECUTABLE dumpbin)
		if(NOT DUMPBIN_EXECUTABLE)
			message(FATAL_ERROR "Trying to find symbol ${symbol} in ${binary_path} but dumpbin was not found")
		endif()
		execute_process(
			COMMAND ${DUMPBIN_EXECUTABLE} /symbols ${binary_path}
			OUTPUT_VARIABLE dumpbin_output
			RESULT_VARIABLE dumpbin_result
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		string(FIND "${dumpbin_output}" symbol SYMBOL_FOUND)
		if(NOT SYMBOL_FOUND EQUAL -1)
			set(${result_var} TRUE PARENT_SCOPE)
		else()
			set(${result_var} FALSE PARENT_SCOPE)
		endif()
	else()
		find_program(NM_EXECUTABLE nm)
		if(NM_EXECUTABLE)
			execute_process(
				COMMAND ${NM_EXECUTABLE} -D ${binary_path}
				OUTPUT_VARIABLE nm_output
				RESULT_VARIABLE nm_result
				ERROR_QUIET
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)
			string(FIND "${nm_output}" symbol SYMBOL_FOUND)
			if(NOT SYMBOL_FOUND EQUAL -1)
				set(${result_var} TRUE PARENT_SCOPE)
			else()
				set(${result_var} FALSE PARENT_SCOPE)
			endif()
		else()
			message(FATAL_ERROR "Trying to find symbol ${symbol} in ${binary_path} but nm was not found")
		endif()
	endif()
endmacro()

function(check_asan_executable binary_path result_var)
	check_symbol_executable("__asan_init" "${binary_path}" ${result_var})
endfunction()

function(check_tsan_executable binary_path result_var)
	check_symbol_executable("__tsan_init" "${binary_path}" ${result_var})
endfunction()

#
# Linker options
#

set(DEFAULT_LINKER_OPTIONS)

if(PROJECT_OS_FAMILY MATCHES "macos" OR PROJECT_OS_LINUX OR PROJECT_OS_MINGW)
	# Enable threads in linux, macos and mingw
	set(DEFAULT_LINKER_OPTIONS
		-pthread
	)
elseif(PROJECT_OS_HAIKU)
	set(DEFAULT_LINKER_OPTIONS
		-lpthread
	)
endif()
