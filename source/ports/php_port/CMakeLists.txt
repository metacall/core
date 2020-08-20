# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_PHP)
	return()
endif()

# TODO: Implement external project (https://github.com/metacall/php-port)
# download the port and package it
