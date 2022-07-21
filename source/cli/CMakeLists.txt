
# Check if CLIs are enabled
if(NOT OPTION_BUILD_CLI)
	return()
endif()

# CLI applications
add_subdirectory(metacallcli)
add_subdirectory(plugins)
