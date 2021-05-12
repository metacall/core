#
# External dependencies
#

set(NODEJS_EXECUTABLE_ONLY ON)

find_package(NodeJS 10.23.1)

if(NOT NODEJS_FOUND)
	message(STATUS "NodeJS executable not found")
	return()
endif()

# Dependencies
add_custom_target(typescript-templating-depends
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/source/templating
	COMMAND npm install
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/templating/package.json ${CMAKE_CURRENT_SOURCE_DIR}/source/templating/package-lock.json
)

set_target_properties(typescript-templating-depends
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

#
# Configure typescript project
#

ts_project(templating 0.1.0)

add_dependencies(typescript-templating typescript-templating-depends)
