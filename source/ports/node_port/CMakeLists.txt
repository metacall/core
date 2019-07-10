# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_NODE)
	return()
endif()

#
# External dependencies
#

find_package(NodeJS 10.15.3)

if(NOT NODEJS_FOUND)
	message(STATUS "NodeJS libraries not found")
	return()
endif()

# MetaCall Library
find_package(MetaCall REQUIRED)

if(NOT METACALL_FOUND)
	message(STATUS "MetaCall libraries not found")
	return()
endif()


#
# Plugin name and options
#

# Target name
set(target node_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

#
# Create target
#

add_custom_target(${target} ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND npm install
	COMMAND npm run configure
	COMMAND npm run build
)

#
# Project options
#

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

#
# Deployment
#

# Library
install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/build/Release/metacall.node
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
