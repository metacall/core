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
# Preprocessor arguments template generation
#

function(preprocessor_arguments_generate _args_size)

	# Configure template variables
	set(PREPROCESSOR_ARGS_SIZE ${_args_size} CACHE STRING "Number of variable arguments supported by preprocessor")

	# Read arguments template headers
	file(READ ${preprocessor_path}/preprocessor_arguments_headers.h.in preprocessor_arguments_headers_in)

	# Read arguments template body
	file(READ ${preprocessor_path}/preprocessor_arguments_body.h.in preprocessor_arguments_body_in)

	# Define arguments template constants
	set(preprocessor_arguments_line_align 10)

	# Macro implementation: PREPROCESSOR_ARGS_COUNT_SEQ_IMPL
	set(PREPROCESSOR_ARGS_COUNT_SEQ_IMPL_BODY "")

	math(EXPR preprocessor_arguments_limit "${PREPROCESSOR_ARGS_SIZE} - 1")

	foreach(iterator RANGE ${preprocessor_arguments_limit} 1)

		math(EXPR iterator_modulo "${iterator} % ${preprocessor_arguments_line_align}")

		if(${iterator_modulo} EQUAL 0)
			set(PREPROCESSOR_ARGS_COUNT_SEQ_IMPL_BODY "${PREPROCESSOR_ARGS_COUNT_SEQ_IMPL_BODY}${iterator}, \\\n\t\t")
		else()
			set(PREPROCESSOR_ARGS_COUNT_SEQ_IMPL_BODY "${PREPROCESSOR_ARGS_COUNT_SEQ_IMPL_BODY}${iterator}, ")
		endif()

	endforeach()

	# Macro implementation: PREPROCESSOR_ARGS_COMMA_SEQ_IMPL
	set(PREPROCESSOR_ARGS_COMMA_SEQ_IMPL_BODY "")

	math(EXPR preprocessor_arguments_limit "${PREPROCESSOR_ARGS_SIZE} - 2")

	foreach(iterator RANGE 1 ${preprocessor_arguments_limit})

		math(EXPR iterator_modulo "${iterator} % ${preprocessor_arguments_line_align}")

		if(${iterator_modulo} EQUAL 0)
			set(PREPROCESSOR_ARGS_COMMA_SEQ_IMPL_BODY "${PREPROCESSOR_ARGS_COMMA_SEQ_IMPL_BODY}1, \\\n\t\t")
		else()
			set(PREPROCESSOR_ARGS_COMMA_SEQ_IMPL_BODY "${PREPROCESSOR_ARGS_COMMA_SEQ_IMPL_BODY}1, ")
		endif()

	endforeach()

	# Macro implementation: PREPROCESSOR_ARGS_N_IMPL
	set(PREPROCESSOR_ARGS_N_IMPL_BODY "")

	math(EXPR preprocessor_arguments_limit "${PREPROCESSOR_ARGS_SIZE} - 1")

	foreach(iterator RANGE 1 ${preprocessor_arguments_limit})

		math(EXPR iterator_modulo "${iterator} % ${preprocessor_arguments_line_align}")

		# Avoid breaking the line after the last argument, the template already does it
		if(${iterator_modulo} EQUAL 0 AND NOT ${iterator} EQUAL ${preprocessor_arguments_limit})
			set(PREPROCESSOR_ARGS_N_IMPL_BODY "${PREPROCESSOR_ARGS_N_IMPL_BODY}_${iterator}, \\\n\t")
		else()
			set(PREPROCESSOR_ARGS_N_IMPL_BODY "${PREPROCESSOR_ARGS_N_IMPL_BODY}_${iterator}, ")
		endif()

	endforeach()

	# Macro implementation: PREPROCESSOR_ARGS_COMMA_IMPL
	set(preprocessor_args_comma_impl_names "")
	set(preprocessor_args_comma_impl_values "")

	math(EXPR preprocessor_arguments_limit "${PREPROCESSOR_ARGS_SIZE}")

	foreach(iterator RANGE 0 ${preprocessor_arguments_limit})

		list(APPEND preprocessor_args_comma_impl_names "PREPROCESSOR_ARGS_COMMA_IMPL_${iterator}")

		if(iterator LESS 2)
			list(APPEND preprocessor_args_comma_impl_values "1")
		else()
			list(APPEND preprocessor_args_comma_impl_values "0")
		endif()

	endforeach()

	preprocessor_template_align(PREPROCESSOR_ARGS_COMMA_IMPL_BODY "\t\t" preprocessor_args_comma_impl_names preprocessor_args_comma_impl_values)

	# Macro implementation: PREPROCESSOR_ARGS_COUNT_PREFIX_POSTFIX_BODY
	set(PREPROCESSOR_ARGS_COUNT_PREFIX_POSTFIX_BODY "")

	math(EXPR preprocessor_arguments_limit "${PREPROCESSOR_ARGS_SIZE} - 1")

	foreach(iterator RANGE 1 ${preprocessor_arguments_limit})

		set(PREPROCESSOR_ARGS_COUNT_PREFIX_POSTFIX_BODY "${PREPROCESSOR_ARGS_COUNT_PREFIX_POSTFIX_BODY}, ")

	endforeach()

	# Configure arguments template headers
	string(CONFIGURE ${preprocessor_arguments_headers_in} preprocessor_arguments_headers @ONLY)

	# Configure arguments template body
	string(CONFIGURE ${preprocessor_arguments_body_in} preprocessor_arguments_body @ONLY)

	# Generate arguments template implementation
	preprocessor_template_generate("${preprocessor_arguments_headers}" "${preprocessor_arguments_body}" "${preprocessor_output_path}" "arguments")

endfunction()
