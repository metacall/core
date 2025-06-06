#
#	Preprocessor Library by Parra Studios
#	A generic header-only preprocessor metaprogramming library.
#
#	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
	set(PREPROCESSOR_INCREMENT_IMPL_BODY "")

	foreach(iterator RANGE 0 ${PREPROCESSOR_ARITHMETIC_SIZE})

		math(EXPR iterator_increment "${iterator} + 1")

		set(PREPROCESSOR_INCREMENT_IMPL_BODY "${PREPROCESSOR_INCREMENT_IMPL_BODY}#define PREPROCESSOR_INCREMENT_IMPL_${iterator} ${iterator_increment}\n")

	endforeach()

	# Definition implementation: PREPROCESSOR_DECREMENT_IMPL
	set(PREPROCESSOR_DECREMENT_IMPL_BODY "")

	foreach(iterator RANGE 1 ${PREPROCESSOR_ARITHMETIC_SIZE})

		math(EXPR iterator_decrement "${iterator} - 1")

		set(PREPROCESSOR_DECREMENT_IMPL_BODY "${PREPROCESSOR_DECREMENT_IMPL_BODY}#define PREPROCESSOR_DECREMENT_IMPL_${iterator} ${iterator_decrement}\n")

	endforeach()

	# Configure arithmetic template headers
	string(CONFIGURE ${preprocessor_arithmetic_headers_in} preprocessor_arithmetic_headers @ONLY)

	# Configure arithmetic template body
	string(CONFIGURE ${preprocessor_arithmetic_body_in} preprocessor_arithmetic_body @ONLY)

	# Generate arithmetic template implementation
	preprocessor_template_generate("${preprocessor_arithmetic_headers}" "${preprocessor_arithmetic_body}" "${preprocessor_output_path}" "arithmetic")

endfunction()
