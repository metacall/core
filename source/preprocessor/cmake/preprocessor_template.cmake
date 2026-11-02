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

#
# Preprocessor template macro alignment
#

# Generates a block of consecutive macro definitions aligned in the same way
# clang-format does with AlignConsecutiveMacros, UseTab: Always and TabWidth: 4
# (see .clang-format), so the generated headers already pass the formatting check.
# The values are aligned one column after the longest definition, padding with
# a tab to the next tab stop, then tabs, then spaces; when the padding is a single
# column or it does not reach the next tab stop, only spaces are used.
#	_result: output variable where the generated block is stored
#	_indent: string of tabs prepended to each definition
#	_names: name of the list variable with the macro definitions (i.e: NAME or NAME(args))
#	_values: name of the list variable with the macro values
function(preprocessor_template_align _result _indent _names _values)

	set(tab_width 4)

	# Obtain the column where the definitions start
	string(LENGTH "${_indent}" indent_length)
	math(EXPR indent_column "${indent_length} * ${tab_width}")

	# Obtain the column after the longest definition
	set(align_column 0)

	foreach(name IN LISTS ${_names})
		string(LENGTH "#define ${name}" name_length)
		math(EXPR name_column "${indent_column} + ${name_length} + 1")

		if(name_column GREATER align_column)
			set(align_column ${name_column})
		endif()
	endforeach()

	# Generate the aligned definitions
	set(result "")
	list(LENGTH ${_names} names_length)
	math(EXPR names_last "${names_length} - 1")

	foreach(index RANGE ${names_last})
		list(GET ${_names} ${index} name)
		list(GET ${_values} ${index} value)

		string(LENGTH "#define ${name}" name_length)
		math(EXPR name_column "${indent_column} + ${name_length}")
		math(EXPR padding "${align_column} - ${name_column}")
		math(EXPR first_tab "${tab_width} - ${name_column} % ${tab_width}")

		if(padding LESS first_tab OR padding EQUAL 1)
			string(REPEAT " " ${padding} padding_str)
		else()
			math(EXPR padding "${padding} - ${first_tab}")
			math(EXPR padding_tabs "${padding} / ${tab_width} + 1")
			math(EXPR padding_spaces "${padding} % ${tab_width}")
			string(REPEAT "\t" ${padding_tabs} padding_tabs_str)
			string(REPEAT " " ${padding_spaces} padding_spaces_str)
			set(padding_str "${padding_tabs_str}${padding_spaces_str}")
		endif()

		string(APPEND result "${_indent}#define ${name}${padding_str}${value}\n")
	endforeach()

	set(${_result} "${result}" PARENT_SCOPE)

endfunction()

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
