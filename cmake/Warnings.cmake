#
#	Cross-compiler warning utility by Parra Studios
#	Utility to enable cross-compiler warnings.
#
#	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
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
		set(WARNINGS_AVAILABLE NULL)
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
		add_compile_options(-Wpedantic)
		add_compile_options(-Wpointer-arith)
		#add_compile_options(-Wcast-qual)
		#add_compile_options(-Winline)
		#add_compile_options(-Wold-style-definition)
		add_compile_options(-Wstrict-aliasing=2)
		#add_compile_options(-Wundef)
		add_compile_options(-Wredundant-decls)

		if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
			add_compile_options(-Wuninitialized)
		else()
			add_compile_options(-Wmaybe-uninitialized)
			add_compile_options(-Wreturn-local-addr)
			add_compile_options(-Wlogical-op)
		endif()

		# C only flags
		#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs")
		#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wbad-function-cast -Wdeclaration-after-statement")

		set(WARNINGS_C_AVAILABLE 1)
	elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
		string(REPLACE "/W1" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
		string(REPLACE "/W2" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
		string(REPLACE "/W3" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /Wall")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D _CTR_NONSTDC_NO_WARNINGS=1")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D _CTR_SECURE_NO_WARNINGS=1")
		set(WARNINGS_C_AVAILABLE 1)
	elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W5 /Wall /Wcheck /Werror-all")
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
		string(REPLACE "/W1" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		string(REPLACE "/W2" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Wall")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D _CTR_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D _CTR_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D _CTR_NONSTDC_NO_WARNINGS=1")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D _CTR_SECURE_NO_WARNINGS=1")
		set(WARNINGS_CXX_AVAILABLE 1)
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W5 /Wall /Wcheck /Werror-all")
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
