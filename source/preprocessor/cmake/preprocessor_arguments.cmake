#
#	Preprocssor Library by Parra Studios
#	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	A generic header-only preprocessor metaprogramming library.
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
			set(PREPROCESSOR_ARGS_COUNT_SEQ_IMPL_BODY "${PREPROCESSOR_ARGS_COUNT_SEQ_IMPL_BODY}${iterator}, \\\n\t")
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
			set(PREPROCESSOR_ARGS_COMMA_SEQ_IMPL_BODY "${PREPROCESSOR_ARGS_COMMA_SEQ_IMPL_BODY}1, \\\n\t")
		else()
			set(PREPROCESSOR_ARGS_COMMA_SEQ_IMPL_BODY "${PREPROCESSOR_ARGS_COMMA_SEQ_IMPL_BODY}1, ")
		endif()

	endforeach()

	# Macro implementation: PREPROCESSOR_ARGS_N_IMPL
	set(PREPROCESSOR_ARGS_N_IMPL_BODY "")

	math(EXPR preprocessor_arguments_limit "${PREPROCESSOR_ARGS_SIZE} - 1")

	foreach(iterator RANGE 1 ${preprocessor_arguments_limit})

		math(EXPR iterator_modulo "${iterator} % ${preprocessor_arguments_line_align}")

		if(${iterator_modulo} EQUAL 0)
			set(PREPROCESSOR_ARGS_N_IMPL_BODY "${PREPROCESSOR_ARGS_N_IMPL_BODY}_${iterator}, \\\n\t")
		else()
			set(PREPROCESSOR_ARGS_N_IMPL_BODY "${PREPROCESSOR_ARGS_N_IMPL_BODY}_${iterator}, ")
		endif()

	endforeach()

	# Macro implementation: PREPROCESSOR_ARGS_COUNT_PREFIX_POSTFIX_BODY
	set(PREPROCESSOR_ARGS_COUNT_PREFIX_POSTFIX_BODY "")

	math(EXPR preprocessor_arguments_limit "${PREPROCESSOR_ARGS_SIZE} - 1")

	foreach(iterator RANGE 1 ${preprocessor_arguments_limit})

		set(PREPROCESSOR_ARGS_COUNT_PREFIX_POSTFIX_BODY "${PREPROCESSOR_ARGS_COUNT_PREFIX_POSTFIX_BODY},")

	endforeach()

	# Configure arguments template headers
	string(CONFIGURE ${preprocessor_arguments_headers_in} preprocessor_arguments_headers @ONLY)

	# Configure arguments template body
	string(CONFIGURE ${preprocessor_arguments_body_in} preprocessor_arguments_body @ONLY)

	# Generate arguments template implementation
	preprocessor_template_generate("${preprocessor_arguments_headers}" "${preprocessor_arguments_body}" "${preprocessor_output_path}" "arguments")

endfunction()
