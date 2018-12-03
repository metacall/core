#
# Script project generator by Parra Studios
# Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

function(script_project name language configuration)

	# Create project file
	string(TOLOWER ${language} language_lower)

	# Create project file
	set(custom_target "${language_lower}-${name}")

	# Create project file
	configure_file(${configuration} ${custom_target}-config.cmake @ONLY)

	# Include generated project file
	include(${CMAKE_CURRENT_BINARY_DIR}/${custom_target}-config.cmake)

	# Set custom target
	add_custom_target(${custom_target} ALL)

	#
	# Deployment
	#

	# Install cmake script config
	#install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/${custom_target}/${custom_target}-config.cmake"
	#	DESTINATION ${INSTALL_CMAKE}/${custom_target}
	#	COMPONENT   runtime
	#)

	# CMake config
	#install(EXPORT  ${custom_target}-export
	#	NAMESPACE   ${META_PROJECT_NAME}::
	#	DESTINATION ${INSTALL_CMAKE}/${custom_target}
	#	COMPONENT   dev
	#)

	# Set project properties
	set_target_properties(${custom_target}
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS}
		FOLDER "${IDE_FOLDER}/${language}"
	)

	# Copy scripts
	add_custom_command(TARGET ${custom_target} PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/source ${LOADER_SCRIPT_PATH}
	)

endfunction()
