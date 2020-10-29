#
# External dependencies
#

set(NODEJS_EXECUTABLE_ONLY ON)

find_package(NodeJS 10.22.0)

if(NOT NODEJS_FOUND)
	message(STATUS "NodeJS executable not found")
	return()
endif()

# Detect if React is available
execute_process(
	COMMAND ${NODEJS_EXECUTABLE} -e "require('react')"
	COMMAND ${NODEJS_EXECUTABLE} -e "require('react-dom')"
	RESULT_VARIABLE NODEJS_REACT_FOUND
	ERROR_QUIET
	OUTPUT_QUIET
)

if("${NODEJS_REACT_FOUND}" EQUAL "1")
	message(WARNING "NodeJS React not found, skipping the TypeScript templating script project")
	return()
endif()


#
# Configure typescript project
#

ts_project(templating 0.1.0)
