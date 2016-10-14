
# Define current preprocessor path
get_filename_component(preprocessor_path ${CMAKE_CURRENT_LIST_FILE} PATH)

# Include preprocessor template generator
include(${preprocessor_path}/preprocessor_template.cmake)

#
# Preprocessor boolean template generation
#

function(preprocessor_boolean_generate _args_size)

	# Configure template variables
	set(PREPROCESSOR_BOOL_SIZE ${_args_size} CACHE STRING "Number of boolean arguments supported by preprocessor")

	# Read boolean template headers
	file(READ ${preprocessor_path}/preprocessor_boolean_headers.h.in preprocessor_boolean_headers_in)

	# Read boolean template body
	file(READ ${preprocessor_path}/preprocessor_boolean_body.h.in preprocessor_boolean_body_in)

	# Definition implementation: PREPROCESSOR_BOOL_IMPL
	set(PREPROCESSOR_BOOL_IMPL_BODY "")

	foreach(iterator RANGE 0 ${PREPROCESSOR_BOOL_SIZE})

		set(PREPROCESSOR_BOOL_IMPL_BODY "${PREPROCESSOR_BOOL_IMPL_BODY}#define PREPROCESSOR_BOOL_IMPL_${iterator} ${iterator}\n")

	endforeach()

	# Configure boolean template headers
	string(CONFIGURE ${preprocessor_boolean_headers_in} preprocessor_boolean_headers @ONLY)

	# Configure boolean template body
	string(CONFIGURE ${preprocessor_boolean_body_in} preprocessor_boolean_body @ONLY)

	# Generate boolean template implementation
	preprocessor_template_generate("${preprocessor_boolean_headers}" "${preprocessor_boolean_body}" "." "boolean")

endfunction()
