#Check if extension loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_EXTENSIONS)
	return()
endif()

# Extension options
option(OPTION_BUILD_EXTENSIONS_LOAD "Build Load Extension." ON)

# Extension sub-projects
add_subdirectory(load_extension)
