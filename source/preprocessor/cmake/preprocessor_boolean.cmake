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
# Preprocessor boolean template generation
#

function(preprocessor_boolean_generate _bool_size)

	# Configure template variables
	set(PREPROCESSOR_BOOL_SIZE ${_bool_size} CACHE STRING "Number of boolean arguments supported by preprocessor")

	# Read boolean template headers
	file(READ ${preprocessor_path}/preprocessor_boolean_headers.h.in preprocessor_boolean_headers_in)

	# Read boolean template body
	file(READ ${preprocessor_path}/preprocessor_boolean_body.h.in preprocessor_boolean_body_in)

	# Definition implementation: PREPROCESSOR_BOOL_IMPL
	set(PREPROCESSOR_BOOL_IMPL_BODY "")

	foreach(iterator RANGE 1 ${PREPROCESSOR_BOOL_SIZE})

		set(PREPROCESSOR_BOOL_IMPL_BODY "${PREPROCESSOR_BOOL_IMPL_BODY}#define PREPROCESSOR_BOOL_IMPL_${iterator} 1\n")

	endforeach()

	# Configure boolean template headers
	string(CONFIGURE ${preprocessor_boolean_headers_in} preprocessor_boolean_headers @ONLY)

	# Configure boolean template body
	string(CONFIGURE ${preprocessor_boolean_body_in} preprocessor_boolean_body @ONLY)

	# Generate boolean template implementation
	preprocessor_template_generate("${preprocessor_boolean_headers}" "${preprocessor_boolean_body}" "${preprocessor_output_path}" "boolean")

endfunction()
