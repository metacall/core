#
# Compile options configuration
#

option(OPTION_BUILD_SANITIZER			"Build with sanitizer compiler options."			OFF)
option(OPTION_BUILD_MEMORY_SANITIZER	"Build with memory sanitizer compiler options."		OFF)
option(OPTION_BUILD_THREAD_SANITIZER	"Build with thread sanitizer compiler options."		OFF)

if((OPTION_BUILD_SANITIZER AND OPTION_BUILD_MEMORY_SANITIZER) OR (OPTION_BUILD_SANITIZER AND OPTION_BUILD_THREAD_SANITIZER) OR (OPTION_BUILD_MEMORY_SANITIZER AND OPTION_BUILD_THREAD_SANITIZER))
	message(FATAL_ERROR "OPTION_BUILD_SANITIZER and OPTION_BUILD_MEMORY_SANITIZER and OPTION_BUILD_THREAD_SANITIZER are mutually exclusive, choose one of them")
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

set(DEFAULT_PROJECT_OPTIONS
	DEBUG_POSTFIX				"d"
	CXX_STANDARD				11 # Not available before CMake 3.1; see below for manual command line argument addition
	LINKER_LANGUAGE				"CXX"
	POSITION_INDEPENDENT_CODE	ON
	CXX_VISIBILITY_PRESET		"hidden"
)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(DEFAULT_PROJECT_OPTIONS
		${DEFAULT_PROJECT_OPTIONS}
		C_STANDARD				99 # TODO: Provide preprocessor support for older standards (GCC)
	)
endif()

#
# Include directories
#

set(DEFAULT_INCLUDE_DIRECTORIES)

#
# Libraries
#

# ThreadSanitizer is incompatible with AddressSanitizer and LeakSanitizer
if(OPTION_BUILD_THREAD_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
	set(DEFAULT_LIBRARIES -ltsan)
	set(TESTS_SANITIZER_ENVIRONMENT_VARIABLES)
	set(SANITIZER_COMPILE_DEFINITIONS)
elseif(OPTION_BUILD_MEMORY_SANITIZER AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
	# TODO: This requires much more effort than expected: https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo
	set(DEFAULT_LIBRARIES)
	set(TESTS_SANITIZER_ENVIRONMENT_VARIABLES)
	set(SANITIZER_COMPILE_DEFINITIONS)
elseif(OPTION_BUILD_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
	set(DEFAULT_LIBRARIES -lasan -lubsan)
	set(TESTS_SANITIZER_ENVIRONMENT_VARIABLES
		"LSAN_OPTIONS=verbosity=1:log_threads=1:print_suppressions=false:suppressions=${CMAKE_SOURCE_DIR}/source/tests/sanitizer/lsan.supp"

		# Specify use_sigaltstack=0 as CoreCLR uses own alternate stack for signal handlers (https://github.com/swgillespie/coreclr/commit/bec020aa466d08e49e007d0011b0e79f8f7c7a62)
		# "ASAN_OPTIONS=use_sigaltstack=0:symbolize=1:alloc_dealloc_mismatch=0:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1"

		# Specify handle_segv=0 and detect_leaks=0 for the JVM (https://blog.gypsyengineer.com/en/security/running-java-with-addresssanitizer.html)
		# "ASAN_OPTIONS=detect_leaks=0:handle_segv=0:symbolize=1:alloc_dealloc_mismatch=0:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1"

		"ASAN_OPTIONS=use_sigaltstack=0:symbolize=1:alloc_dealloc_mismatch=0:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1"
		"UBSAN_OPTIONS=print_stacktrace=1"
	)
	set(SANITIZER_COMPILE_DEFINITIONS
		"__ADDRESS_SANITIZER__=1"
	)
else()
	set(DEFAULT_LIBRARIES)
	set(TESTS_SANITIZER_ENVIRONMENT_VARIABLES)
	set(SANITIZER_COMPILE_DEFINITIONS)
endif()

#
# Compile definitions
#

if(OPTION_BUILD_LOG_PRETTY)
	set(LOG_POLICY_FORMAT_PRETTY_VALUE 1)
else()
	set(LOG_POLICY_FORMAT_PRETTY_VALUE 0)
endif()

set(DEFAULT_COMPILE_DEFINITIONS
	LOG_POLICY_FORMAT_PRETTY=${LOG_POLICY_FORMAT_PRETTY_VALUE}
	SYSTEM_${SYSTEM_NAME_UPPER}
	${SANITIZER_COMPILE_DEFINITIONS}
)

# MSVC compiler options
if(WIN32 AND MSVC)
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

	# Release
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
		add_compile_options(/O2)
		add_compile_options(/Oi)
		add_compile_options(/Oy)

		if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
			# Enable debug symbols
			add_compile_options(/Z7)
		endif()
	endif()
endif()

if (PROJECT_OS_FAMILY MATCHES "unix")

	if(APPLE)
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

	# Debug symbols
	if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		add_compile_options(-g)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
	endif()

	# Optimizations
	if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		add_compile_options(-O3)
	endif()

	# Sanitizers
	if(OPTION_BUILD_THREAD_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		add_compile_options(-fno-omit-frame-pointer)
		add_compile_options(-fno-optimize-sibling-calls)
		add_compile_options(-fsanitize=thread)
	elseif(OPTION_BUILD_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		add_compile_options(-fuse-ld=gold)
		add_compile_options(-fno-omit-frame-pointer)
		add_compile_options(-fno-optimize-sibling-calls)
		add_compile_options(-fsanitize=undefined)
		add_compile_options(-fsanitize=address)
		add_compile_options(-fsanitize=leak)
		add_compile_options(-fsanitize-address-use-after-scope)
	elseif(OPTION_BUILD_MEMORY_SANITIZER AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		add_compile_options(-fno-omit-frame-pointer)
		add_compile_options(-fno-optimize-sibling-calls)
		add_compile_options(-fsanitize=undefined)
		add_compile_options(-fsanitize=memory)
		add_compile_options(-fsanitize-memory-track-origins)
		add_compile_options(-fsanitize-memory-use-after-dtor)
	endif()
endif()

#
# Linker options
#

set(DEFAULT_LINKER_OPTIONS)

if(APPLE OR PROJECT_OS_LINUX OR MINGW)
	# Enable threads in linux, macos and mingw
	set(DEFAULT_LINKER_OPTIONS
		-pthread
	)
elseif(PROJECT_OS_HAIKU)
	set(DEFAULT_LINKER_OPTIONS
		-lpthread
	)
endif()
