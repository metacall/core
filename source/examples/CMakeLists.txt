
# Check if examples are enabled
if(NOT OPTION_BUILD_EXAMPLES)
	return()
endif()

# Example applications
add_subdirectory(metacallcli)
add_subdirectory(metacallppcli)
add_subdirectory(metacallgui)
add_subdirectory(metacallweb)
add_subdirectory(metacallnginx)
add_subdirectory(metacallpy)
