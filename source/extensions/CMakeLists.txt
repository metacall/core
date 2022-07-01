#Check if extension loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_EXTENSIONS)
	return()
endif()

# Extension options
option(OPTION_BUILD_EXTENSIONS_PLUGIN "Build plugin extension." ON)

# Extension sub-projects
add_subdirectory(plugin_extension)
