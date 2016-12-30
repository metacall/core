#
# Cross-compiler warning utility by Parra Studios
# Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Defines maximun warning levels for multiple compilers
#
# WARNINGS_INCLUDE - True if warnings are already included
# WARNINGS_AVAILABLE - True if warning levels are supported by the current compiler
# WARNINGS_C_AVAILABLE - True if warning levels are supported by the current C compiler
# WARNINGS_CXX_AVAILABLE - True if warning levels are supported by the current CXX compiler
# WARNINGS_ENABLED - Custom option for user configuration

if(WARNINGS_INCLUDED)
	return()
endif()

# Define module include
set(WARNINGS_INCLUDED 1)

# Define module options
option(WARNINGS_ENABLED "Enables cross-compiler warning support." ON)

if(WARNINGS_ENABLED)

	# Test against a valid compiler
	if("${CMAKE_C_COMPILER_ID}" STREQUAL "" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "")
		message(WARNING "Unknown compiler warning level support (skipping warning module)")
		set(WARNINGS_AVAILABLE 0)
		return()
	endif()

	# Define C compiler warning flags
	if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
		add_compile_options(-Wall)
		add_compile_options(-Wextra)
		add_compile_options(-Wunused)
		#add_compile_options(-Wshadow)
		add_compile_options(-Wignored-qualifiers)
		add_compile_options(-Wmissing-braces)
		add_compile_options(-Wreturn-type)
		#add_compile_options(-Wswitch)
		#add_compile_options(-Wswitch-default)
		#add_compile_options(-Wswitch-enum)
		add_compile_options(-Wstrict-overflow=5)
		add_compile_options(-Wuninitialized)
		add_compile_options(-Wmissing-field-initializers)
		add_compile_options(-Wmaybe-uninitialized)
		add_compile_options(-Wmaybe-uninitialized)
		add_compile_options(-Wpedantic)
		add_compile_options(-Wreturn-local-addr)
		add_compile_options(-Wpointer-arith)
		#add_compile_options(-Wcast-qual)
		#add_compile_options(-Winline)
		#add_compile_options(-Wold-style-definition)
		add_compile_options(-Wstrict-aliasing=2)
		#add_compile_options(-Wundef)
		add_compile_options(-Wlogical-op)
		add_compile_options(-Wredundant-decls)

		# C only flags
		#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs")
		#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wbad-function-cast -Wdeclaration-after-statement")

		set(WARNINGS_C_AVAILABLE 1)
	elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /Wall /WX")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D _CTR_NONSTDC_NO_WARNINGS=1")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D _CTR_SECURE_NO_WARNINGS=1")
		set(WARNINGS_C_AVAILABLE 1)
	elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W5 /Wall /Wcheck /Werror-all /WX")
		set(WARNINGS_C_AVAILABLE 1)
	else()
		set(STATUS "Unknown C compiler warning level support")
		set(WARNINGS_C_AVAILABLE 0)
	endif()

	# Define CXX compiler warning flags
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		set(WARNINGS_CXX_AVAILABLE 1)
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Wall /WX")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D _CTR_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D _CTR_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D _CTR_NONSTDC_NO_WARNINGS=1")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D _CTR_SECURE_NO_WARNINGS=1")
		set(WARNINGS_CXX_AVAILABLE 1)
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W5 /Wall /Wcheck /Werror-all /WX")
		set(WARNINGS_CXX_AVAILABLE 1)
	else()
		set(STATUS "Unknown C++ compiler warning level support")
		set(WARNINGS_CXX_AVAILABLE 0)
	endif()

	# Define warning availability
	if(WARNINGS_C_AVAILABLE OR WARNINGS_CXX_AVAILABLE)
		set(WARNINGS_AVAILABLE 1)
	endif()

endif()
