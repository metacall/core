#
# Script project generator by Parra Studios
# Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Generates a script project embedded into CMake
#

if(SCRIPTPROJECT_FOUND)
    return()
endif()

set(SCRIPTPROJECT_FOUND YES)

# Define current script project configuration path
get_filename_component(SCRIPT_PROJECT_CONFIG_PATH ${CMAKE_CURRENT_LIST_FILE} PATH)

#
# Script sub-project util function
#

function(script_project_capitalize name result)

	# Get first letter
	string(SUBSTRING ${name} 0 1 first_letter)

	# Capitalize first letter
	string(TOUPPER ${first_letter} first_letter_upper)

	# Get remainder
	string(SUBSTRING ${name} 1 -1 remainder)

	# Generate output
	string(CONCAT capitalized_name ${first_letter_upper} ${remainder})

	# Set value on parent scope
	set(${result} ${capitalized_name} PARENT_SCOPE)

endfunction()

function(script_project target language configuration)

    # Create project file
    configure_file(${configuration} ${language}-${target}-config.cmake @ONLY)

    # Include generated project file
    include(${CMAKE_CURRENT_BINARY_DIR}/${language}-${target}-config.cmake)

	# Set custom target
	add_custom_target(${target} ALL)

	#
	# Deployment
	#

	# Install cmake script config
	install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/${target}/${language}-${target}-config.cmake"
		DESTINATION ${INSTALL_CMAKE}/${target}
		COMPONENT   runtime
	)

	# CMake config
	#install(EXPORT  ${target}-export
	#    NAMESPACE   ${META_PROJECT_NAME}::
	#    DESTINATION ${INSTALL_CMAKE}/${target}
	#    COMPONENT   dev
	#)

	# Set project properties
	script_project_capitalize(${language} language_capitalized)

	set_target_properties(${target}
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS}
		FOLDER "${IDE_FOLDER}/${language_capitalized}"
	)

	# Copy scripts
	add_custom_command(TARGET ${target} PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/source ${CMAKE_BINARY_DIR}/scripts
	)

endfunction()
