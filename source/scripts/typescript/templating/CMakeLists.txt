#
# External dependencies
#

find_package(NPM)

if(NOT NPM_FOUND)
	message(STATUS "NPM not found")
	return()
endif()

# Dependencies
add_custom_target(typescript-templating-depends
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/source/templating
	COMMAND ${NPM_EXECUTABLE} install
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
