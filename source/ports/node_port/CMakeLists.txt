# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_NODE)
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
# Install command
#

if(NOT OPTION_BUILD_GUIX)
	execute_process(
		COMMAND npm install
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
endif()

# Files
install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/index.js
	${CMAKE_CURRENT_SOURCE_DIR}/addon.js
	${CMAKE_CURRENT_SOURCE_DIR}/package.json
	${CMAKE_CURRENT_SOURCE_DIR}/package-lock.json
	${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
	DESTINATION ${INSTALL_LIB}/node_modules/metacall
	COMPONENT runtime
)

#
# Configure test
#

# Check if loaders are enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_SCRIPTS_RB)
	return()
endif()

set(node_port_test "${target}_test")

#
# Define test
#

add_test(NAME ${target}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND npm test
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${node_port_test}
)

include(TestEnvironmentVariables)

# Enable cobol test if it is built
if(OPTION_BUILD_LOADERS_COB)
	set(TESTS_ENVIRONMENT_VARIABLES_COB "OPTION_BUILD_LOADERS_COB=1")
endif()

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	${TESTS_ENVIRONMENT_VARIABLES_COB}
)
