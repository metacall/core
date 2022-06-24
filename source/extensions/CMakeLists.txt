#Check if extension loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT)
	return()
endif()

#
# Sub-projects
#

add_subdirectory(load_extensions)
