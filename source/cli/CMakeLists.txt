
# Check if CLIs are enabled
if(NOT OPTION_BUILD_CLI)
	return()
endif()

#add option to build metacall-bootstrap
option(OPTION_BUILD_METACALL_BOOTSTRAP "Build Metacallcli refactor" OFF)

# CLI applications
add_subdirectory(metacallcli)
add_subdirectory(metacallcli-bootstrap) #does nothing for now
