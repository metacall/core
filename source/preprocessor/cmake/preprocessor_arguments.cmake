
# Define current preprocessor path
get_filename_component(preprocessor_path ${CMAKE_CURRENT_LIST_FILE} PATH)

# Include preprocessor template generator
include(${preprocessor_path}/preprocessor_template.cmake)

#
# Preprocessor arguments template generation
#

function(preprocessor_arguments_generate _args_size)

	# Configure template variables
	set(PREPROCESSOR_ARGUMENTS_SIZE ${_args_size} CACHE STRING "Number of variable arguments supported by preprocessor")

	# Read arguments template headers
	file(READ ${preprocessor_path}/preprocessor_arguments_headers.h.in preprocessor_arguments_headers_in)

	# Read arguments template body
	file(READ ${preprocessor_path}/preprocessor_arguments_body.h.in preprocessor_arguments_body_in)

	# Define arguments template constants
	set(preprocessor_arguments_line_align 10)

	# Macro implementation: PREPROCESSOR_ARGUMENTS_COUNT_SEQ_IMPL
	set(PREPROCESSOR_ARGUMENTS_COUNT_SEQ_IMPL_BODY "")

	math(EXPR preprocessor_arguments_limit "${PREPROCESSOR_ARGUMENTS_SIZE} - 1")

	foreach(iterator RANGE ${preprocessor_arguments_limit} 1)

		math(EXPR iterator_modulo "${iterator} % ${preprocessor_arguments_line_align}")

		if(${iterator_modulo} EQUAL 0)
			set(PREPROCESSOR_ARGUMENTS_COUNT_SEQ_IMPL_BODY "${PREPROCESSOR_ARGUMENTS_COUNT_SEQ_IMPL_BODY}${iterator}, \\\n\t")
		else()
			set(PREPROCESSOR_ARGUMENTS_COUNT_SEQ_IMPL_BODY "${PREPROCESSOR_ARGUMENTS_COUNT_SEQ_IMPL_BODY}${iterator}, ")
		endif()

	endforeach()

	# Macro implementation: PREPROCESSOR_ARGUMENTS_COMMA_SEQ_IMPL
	set(PREPROCESSOR_ARGUMENTS_COMMA_SEQ_IMPL_BODY "")

	math(EXPR preprocessor_arguments_limit "${PREPROCESSOR_ARGUMENTS_SIZE} - 2")

	foreach(iterator RANGE 1 ${preprocessor_arguments_limit})

		math(EXPR iterator_modulo "${iterator} % ${preprocessor_arguments_line_align}")

		if(${iterator_modulo} EQUAL 0)
			set(PREPROCESSOR_ARGUMENTS_COMMA_SEQ_IMPL_BODY "${PREPROCESSOR_ARGUMENTS_COMMA_SEQ_IMPL_BODY}1, \\\n\t")
		else()
			set(PREPROCESSOR_ARGUMENTS_COMMA_SEQ_IMPL_BODY "${PREPROCESSOR_ARGUMENTS_COMMA_SEQ_IMPL_BODY}1, ")
		endif()

	endforeach()

	# Macro implementation: PREPROCESSOR_ARGUMENTS_N_IMPL
	set(PREPROCESSOR_ARGUMENTS_N_IMPL_BODY "")

	math(EXPR preprocessor_arguments_limit "${PREPROCESSOR_ARGUMENTS_SIZE} - 1")

	foreach(iterator RANGE 1 ${preprocessor_arguments_limit})

		math(EXPR iterator_modulo "${iterator} % ${preprocessor_arguments_line_align}")

		if(${iterator_modulo} EQUAL 0)
			set(PREPROCESSOR_ARGUMENTS_N_IMPL_BODY "${PREPROCESSOR_ARGUMENTS_N_IMPL_BODY}_${iterator}, \\\n\t")
		else()
			set(PREPROCESSOR_ARGUMENTS_N_IMPL_BODY "${PREPROCESSOR_ARGUMENTS_N_IMPL_BODY}_${iterator}, ")
		endif()

	endforeach()












	# ...



	# Configure arguments template headers
	string(CONFIGURE ${preprocessor_arguments_headers_in} preprocessor_arguments_headers @ONLY)

	# Configure arguments template body
	string(CONFIGURE ${preprocessor_arguments_body_in} preprocessor_arguments_body @ONLY)

	# Generate arguments template implementation
	preprocessor_template_generate("${preprocessor_arguments_headers}" "${preprocessor_arguments_body}" "." "arguments")

endfunction()
