#
# Compile options configuration
#

option(OPTION_BUILD_SANITIZER	"Build with sanitizer compiler options."		OFF)

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

if(OPTION_BUILD_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
	set(DEFAULT_LIBRARIES -lasan -lubsan)
else()
	set(DEFAULT_LIBRARIES)
endif()

#
# Compile definitions
#

set(DEFAULT_COMPILE_DEFINITIONS
	SYSTEM_${SYSTEM_NAME_UPPER}
)

# MSVC compiler options
if(WIN32)
	set(DEFAULT_COMPILE_DEFINITIONS ${DEFAULT_COMPILE_DEFINITIONS}
		_SCL_SECURE_NO_WARNINGS	# Calling any one of the potentially unsafe methods in the Standard C++ Library
		_CRT_SECURE_NO_WARNINGS	# Calling any one of the potentially unsafe methods in the CRT Library
	)
endif ()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR MAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
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

if(WIN32)
	add_compile_options(/nologo) # Suppress Startup Banner
	add_compile_options(/W4) # Set warning level to 4
	add_compile_options(/WX-) # Do not treat warnings as errors
	add_compile_options(/Gm-) # Disable minimal rebuild
	add_compile_options(/MP) # Build with Multiple Processes (number of processes equal to the number of processors)
	#add_compile_options(/wd4251 /wd4592)
	#add_compile_options(/ZH:SHA_256) # use SHA256 for generating hashes of compiler processed source files.

	# Release
	if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		# Disable optimizations
		add_compile_options(/Od)
	else()
		# Enable read-only string pooling
		add_compile_options(/GF)

		# Buffer Security Check
		add_compile_options(/GS)

		# Enable optimizations
		add_compile_options(/O2)
		add_compile_options(/Oi)
		add_compile_options(/Oy)
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

	if(PROJECT_OS_LINUX)
		# Enable threads in linux
		add_compile_options(-pthread)
	endif()

	# All warnings that are not explicitly disabled are reported as errors
	#add_compile_options(-Werror)
	add_compile_options(-Wall)
	add_compile_options(-Wextra)

	if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		add_compile_options(-g)
	else()
		add_compile_options(-O3)
	endif()

	# Sanitizers
	if(OPTION_BUILD_SANITIZER AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		add_compile_options(-fuse-ld=gold)
		add_compile_options(-fno-omit-frame-pointer)
		add_compile_options(-fno-optimize-sibling-calls)
		add_compile_options(-fsanitize=undefined)
		add_compile_options(-fsanitize=address)
		add_compile_options(-fsanitize=leak)
		#add_compile_options(-fsanitize=thread)

		if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
			add_compile_options(-fsanitize=memory)
			add_compile_options(-fsanitize=memory-track-origins)
		endif()
	endif()
endif()

#
# Linker options
#

set(DEFAULT_LINKER_OPTIONS)

# Use pthreads on mingw and linux
if(("${CMAKE_C_COMPILER_ID}" MATCHES "GNU" AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU") OR "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	set(DEFAULT_LINKER_OPTIONS
		-pthread
	)
endif()
