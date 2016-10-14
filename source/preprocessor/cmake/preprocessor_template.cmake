#
#	Preprocssor Library by Parra Studios
#	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	A generic header-only preprocessor metaprogramming library.
#

# Define current preprocessor path
get_filename_component(preprocessor_path ${CMAKE_CURRENT_LIST_FILE} PATH)

#
# Preprocessor template generation
#

function(preprocessor_template_generate _headers _body _path _name)

	# Configure template variables
	if(_headers STREQUAL "")
		set(PREPROCESSOR_TEMPLATE_HEADERS "${_headers}")
	else()
		set(PREPROCESSOR_TEMPLATE_HEADERS "\n${_headers}")
	endif()

	set(PREPROCESSOR_TEMPLATE_BODY ${_body})

	set(PREPROCESSOR_TEMPLATE_NAME ${_name})

	string(TOUPPER "PREPROCESSOR_${_name}_H" PREPROCESSOR_TEMPLATE_NAME_HEADER)

	# Generate output template implementation
	set(preprocessor_output_file "${_path}/preprocessor_${_name}.h")

	configure_file(${preprocessor_path}/preprocessor_template.h.in ${preprocessor_output_file})

endfunction()
