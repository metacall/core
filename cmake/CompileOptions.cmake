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
else()
	set(MEMORYCHECK_COMPILE_DEFINITIONS)
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
		# foreach(FLAG_VAR
		# 	CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE
		# 	CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
		# 	CMAKE_C_FLAGS CMAKE_C_FLAGS_RELEASE
		# 	CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
		# 	)
		# 	string(REGEX REPLACE "/RTC[^ ]*" "" ${FLAG_VAR} "${${FLAG_VAR}}")
		# endforeach(FLAG_VAR)

		if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
			# Enable debug symbols
			add_compile_options(/Z7)
		endif()
	endif()

	# Sanitizers
	if(OPTION_BUILD_THREAD_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		add_compile_options(/fsanitize=thread)
		# add_compile_options(/fsanitize=undefined)
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
