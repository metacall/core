#
#	Preprocessor Library by Parra Studios
#	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
# Preprocessor for template generation
#

function(preprocessor_for_generate _for_size)

	# Configure template variables
	set(PREPROCESSOR_FOR_SIZE ${_for_size} CACHE STRING "Number of for iterations supported by preprocessor")

	# Read for template headers
	file(READ ${preprocessor_path}/preprocessor_for_headers.h.in preprocessor_for_headers_in)

	# Read for template body
	file(READ ${preprocessor_path}/preprocessor_for_body.h.in preprocessor_for_body_in)

	# Define for template constants
	set(preprocessor_for_line_align 10)

	# Definition implementation: PREPROCESSOR_FOR_EACH_IMPL
	set(PREPROCESSOR_FOR_EACH_IMPL_BODY "")

	foreach(iterator RANGE 3 ${PREPROCESSOR_FOR_SIZE})

		math(EXPR prev "${iterator} - 1")

		set(PREPROCESSOR_FOR_EACH_IMPL_BODY "${PREPROCESSOR_FOR_EACH_IMPL_BODY}#define PREPROCESSOR_FOR_EACH_IMPL_${iterator}(expr, element, ...) expr(element)")

		set(PREPROCESSOR_FOR_EACH_IMPL_BODY "${PREPROCESSOR_FOR_EACH_IMPL_BODY} PREPROCESSOR_FOR_EACH_EVAL(PREPROCESSOR_FOR_EACH_IMPL_${prev}(expr, __VA_ARGS__))\n")

	endforeach()

	# Definition implementation: PREPROCESSOR_FOR_EACH_IMPL_GNUC
	set(PREPROCESSOR_FOR_EACH_IMPL_GNUC_BODY "")

	math(EXPR preprocessor_for_limit "${PREPROCESSOR_FOR_SIZE} - 1")

	foreach(iterator RANGE ${preprocessor_for_limit} 1)

		math(EXPR iterator_modulo "${iterator} % ${preprocessor_for_line_align}")

		if(${iterator_modulo} EQUAL 0)
			set(PREPROCESSOR_FOR_EACH_IMPL_GNUC_BODY "${PREPROCESSOR_FOR_EACH_IMPL_GNUC_BODY}PREPROCESSOR_FOR_EACH_IMPL_${iterator}, \\\n\t\t")
		else()
			set(PREPROCESSOR_FOR_EACH_IMPL_GNUC_BODY "${PREPROCESSOR_FOR_EACH_IMPL_GNUC_BODY}PREPROCESSOR_FOR_EACH_IMPL_${iterator}, ")
		endif()

	endforeach()

	# Configure for template headers
	string(CONFIGURE ${preprocessor_for_headers_in} preprocessor_for_headers @ONLY)

	# Configure for template body
	string(CONFIGURE ${preprocessor_for_body_in} preprocessor_for_body @ONLY)

	# Generate for template implementation
	preprocessor_template_generate("${preprocessor_for_headers}" "${preprocessor_for_body}" "${preprocessor_output_path}" "for")

endfunction()
