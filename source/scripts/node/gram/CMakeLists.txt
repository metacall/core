#
# External dependencies
#

set(NODEJS_EXECUTABLE_ONLY ON)

find_package(NodeJS)

if(NOT NODEJS_FOUND)
	message(STATUS "NodeJS executable not found")
	return()
endif()

# Dependencies
add_custom_target(nodejs-gram-depends
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/source/gram
	COMMAND npm install
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/gram/package.json ${CMAKE_CURRENT_SOURCE_DIR}/source/gram/package-lock.json
)

set_target_properties(nodejs-gram-depends
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

#
# Configure nodejs project
#

nodejs_project(gram 0.1.0)

add_dependencies(nodejs-gram nodejs-gram-depends)
