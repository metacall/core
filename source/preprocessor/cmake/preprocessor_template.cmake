#
#	Preprocessor Library by Parra Studios
#	A generic header-only preprocessor metaprogramming library.
#
#	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

	set(PREPROCESSOR_TEMPLATE_BODY "${_body}")

	set(PREPROCESSOR_TEMPLATE_NAME "${_name}")

	string(TOUPPER "PREPROCESSOR_${_name}_H" PREPROCESSOR_TEMPLATE_NAME_HEADER)

	# Generate output template implementation
	set(preprocessor_output_file "${_path}/preprocessor_${_name}.h")

	# Read template
	file(READ "${preprocessor_path}/preprocessor_template.h.in" preprocessor_template_in)

	# Configure template string
	string(CONFIGURE "${preprocessor_template_in}" preprocessor_output_file_str)

	# Check against differences
	file(READ "${preprocessor_output_file}" preprocessor_prev_output_file_str)

	# Write output file if there are differences
	if(NOT "${preprocessor_output_file_str}" STREQUAL "${preprocessor_prev_output_file_str}")
		file(WRITE "${preprocessor_output_file}" "${preprocessor_output_file_str}")
	endif()

endfunction()
