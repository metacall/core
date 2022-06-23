# Check if this script is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_EXT)
	return()
endif()

#
# Sub-projects
#

add_subdirectory(sum_extension)
