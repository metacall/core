#
#	CMake SimdJSON Options by Parra Studios
#	CMake script to generate the SimdJSON compile options.
#
#	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# Some of the SimdJSON definitions are used by the headers (i.e: SIMDJSON_UTF8VALIDATION,
# SIMDJSON_IMPLEMENTATION_*), so the library and the code including it must be compiled
# with the same definitions, this script generates them from the options.
#
# The following variables are set:
#
# SIMDJSON_COMPILE_DEFINITIONS - List of compile definitions.
# SIMDJSON_CXX_FLAGS - Custom SimdJSON compilation flags.

set(SIMDJSON_ALL_IMPLEMENTATIONS fallback westmere haswell icelake arm64 ppc64)

set(SIMDJSON_COMPILE_DEFINITIONS)

# Do not validate UTF-8 by default, in order to behave the same as RapidJSON
if(SIMDJSON_UTF8VALIDATION)
	list(APPEND SIMDJSON_COMPILE_DEFINITIONS "SIMDJSON_UTF8VALIDATION=1")
else()
	list(APPEND SIMDJSON_COMPILE_DEFINITIONS "SIMDJSON_UTF8VALIDATION=0")
endif()

# Select the implementations, by default all the supported ones are included and selected at runtime
if(SIMDJSON_IMPLEMENTATION)
	foreach(impl IN LISTS SIMDJSON_IMPLEMENTATION)
		if(NOT impl IN_LIST SIMDJSON_ALL_IMPLEMENTATIONS)
			message(FATAL_ERROR "SimdJSON implementation '${impl}' is not supported, use one of: ${SIMDJSON_ALL_IMPLEMENTATIONS}")
		endif()
	endforeach()

	foreach(impl IN LISTS SIMDJSON_ALL_IMPLEMENTATIONS)
		string(TOUPPER "${impl}" impl_upper)

		if(impl IN_LIST SIMDJSON_IMPLEMENTATION)
			list(APPEND SIMDJSON_COMPILE_DEFINITIONS "SIMDJSON_IMPLEMENTATION_${impl_upper}=1")
		else()
			list(APPEND SIMDJSON_COMPILE_DEFINITIONS "SIMDJSON_IMPLEMENTATION_${impl_upper}=0")
		endif()
	endforeach()
endif()

if(SIMDJSON_EXTRA_DEFINITIONS)
	list(APPEND SIMDJSON_COMPILE_DEFINITIONS ${SIMDJSON_EXTRA_DEFINITIONS})
endif()

set(SIMDJSON_CXX_FLAGS "${SIMDJSON_EXTRA_CXX_FLAGS}")

mark_as_advanced(SIMDJSON_COMPILE_DEFINITIONS SIMDJSON_CXX_FLAGS)
