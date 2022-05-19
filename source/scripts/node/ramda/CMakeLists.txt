#
# External dependencies
#

find_package(NPM)

if(NOT NPM_FOUND)
	message(STATUS "NPM not found")
	return()
endif()

# Dependencies
add_custom_target(nodejs-ramda-depends
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/source/ramda
	COMMAND ${NPM_EXECUTABLE} install
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/ramda/package.json ${CMAKE_CURRENT_SOURCE_DIR}/source/ramda/package-lock.json
)

set_target_properties(nodejs-ramda-depends
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

#
# Configure nodejs project
#

nodejs_project(ramda 0.1.0)

add_dependencies(nodejs-ramda nodejs-ramda-depends)
