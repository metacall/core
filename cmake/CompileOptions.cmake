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
		C_STANDARD			99 # TODO: Provide preprocessor support for older standards (GCC)
	)
endif()

#
# Include directories
#

set(DEFAULT_INCLUDE_DIRECTORIES)

#
# Libraries
#

set(DEFAULT_LIBRARIES)

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
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		# TODO: Review debug optimization
		#add_compile_options(/GL) # Enable debugging information
		##add_compile_options(/LTCG) # Enable debugging information
	else()
		add_compile_options(/GS) # Buffer Security Check
		add_compile_options(/GF) # Enable read-only string pooling
		#add_compile_options(/GW) # Enable read-only string pooling
	endif()
endif()

if (PROJECT_OS_FAMILY MATCHES "unix")

	if(APPLE)
		# We cannot enable "stack-protector-strong" On OS X due to a bug in clang compiler (current version 7.0.2)
		if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
			add_compile_options(-fstack-protector)
		endif()

		# Enable threads in OS X
		add_compile_options(-pthread)

		# clang options only
		add_compile_options(-Wreturn-stack-address)
	else()
		if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
			add_compile_options(-fstack-protector-strong)
		endif()
	endif()

	if(PROJECT_OS_LINUX)
		# Enable threads in linux
		add_compile_options(-pthread)
	endif()

	# All warnings that are not explicitly disabled are reported as errors
	#add_compile_options(-Werror)
endif()

#
# Linker options
#

set(DEFAULT_LINKER_OPTIONS)

# Use pthreads on mingw and linux
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	set(DEFAULT_LINKER_OPTIONS
		-pthread
	)
endif()
