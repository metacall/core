
# Check if CLIs are enabled
if(NOT OPTION_BUILD_CLI)
	return()
endif()

# Check if the dependency loaders are enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_EXTENSIONS OR NOT OPTION_BUILD_LOADERS_NODE)
	message(WARNING "The Extension and NodeJS Loaders are a dependency of the CLI, in order to compile the CLI, enable them with -DOPTION_BUILD_LOADERS_EXT=ON -DOPTION_BUILD_LOADERS_NODE=ON")
	return()
endif()

# CLI applications
add_subdirectory(metacallcli)
add_subdirectory(plugins)
