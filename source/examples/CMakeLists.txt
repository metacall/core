
# Check if examples are enabled
if(NOT OPTION_BUILD_EXAMPLES)
	return()
endif()

# Example applications
# add_subdirectory(metacallgui)
# add_subdirectory(metacallweb)
add_subdirectory(metacallquine)
add_subdirectory(metacalllog)
