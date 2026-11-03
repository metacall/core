#
#	Preprocessor Library by Parra Studios
#	A generic header-only preprocessor metaprogramming library.
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

# Define current preprocessor path
get_filename_component(preprocessor_path ${CMAKE_CURRENT_LIST_FILE} PATH)

# Define output preprocessor path
set(preprocessor_output_path "${CMAKE_CURRENT_SOURCE_DIR}/include/preprocessor")

# Include preprocessor template generator
include(${preprocessor_path}/preprocessor_template.cmake)

#
# Preprocessor arithmetic template generation
#

function(preprocessor_arithmetic_generate _arithmetic_size)

	# Configure template variables
	set(PREPROCESSOR_ARITHMETIC_SIZE ${_arithmetic_size} CACHE STRING "Number of arithmetic operations supported by preprocessor")

	# Read arithmetic template headers
	file(READ ${preprocessor_path}/preprocessor_arithmetic_headers.h.in preprocessor_arithmetic_headers_in)

	# Read arithmetic template body
	file(READ ${preprocessor_path}/preprocessor_arithmetic_body.h.in preprocessor_arithmetic_body_in)

	# Definition implementation: PREPROCESSOR_INCREMENT_IMPL
	set(preprocessor_increment_impl_names "")
	set(preprocessor_increment_impl_values "")

	foreach(iterator RANGE 0 ${PREPROCESSOR_ARITHMETIC_SIZE})

		math(EXPR iterator_increment "${iterator} + 1")

		list(APPEND preprocessor_increment_impl_names "PREPROCESSOR_INCREMENT_IMPL_${iterator}")
		list(APPEND preprocessor_increment_impl_values "${iterator_increment}")

	endforeach()

	preprocessor_template_align(PREPROCESSOR_INCREMENT_IMPL_BODY "" preprocessor_increment_impl_names preprocessor_increment_impl_values)

	# Definition implementation: PREPROCESSOR_DECREMENT_IMPL
	set(preprocessor_decrement_impl_names "")
	set(preprocessor_decrement_impl_values "")

	foreach(iterator RANGE 0 ${PREPROCESSOR_ARITHMETIC_SIZE})

		if(iterator EQUAL 0)
			set(iterator_decrement 0)
		else()
			math(EXPR iterator_decrement "${iterator} - 1")
		endif()

		list(APPEND preprocessor_decrement_impl_names "PREPROCESSOR_DECREMENT_IMPL_${iterator}")
		list(APPEND preprocessor_decrement_impl_values "${iterator_decrement}")

	endforeach()

	preprocessor_template_align(PREPROCESSOR_DECREMENT_IMPL_BODY "" preprocessor_decrement_impl_names preprocessor_decrement_impl_values)

	# Configure arithmetic template headers
	string(CONFIGURE ${preprocessor_arithmetic_headers_in} preprocessor_arithmetic_headers @ONLY)

	# Configure arithmetic template body
	string(CONFIGURE ${preprocessor_arithmetic_body_in} preprocessor_arithmetic_body @ONLY)

	# Generate arithmetic template implementation
	preprocessor_template_generate("${preprocessor_arithmetic_headers}" "${preprocessor_arithmetic_body}" "${preprocessor_output_path}" "arithmetic")

endfunction()
