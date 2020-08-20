# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_D)
	return()
endif()

# TODO: Implement external project (https://github.com/metacall/dlang-port)
# download the port and build it
