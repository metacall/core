# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_LOADERS_TS)
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target ts_loader_bootstrap)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target} bootstrap")

#
# Create target
#

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(sources
	${source_path}/bootstrap.ts
)

# Group source files
set(source_group "Source Files")
source_group_by_path(${source_path}  "\\\\.ts$"
	${source_group} ${sources})

if(NOT OPTION_BUILD_GUIX)
	find_package(NPM)

	if(NOT NPM_FOUND)
		message(SEND_ERROR "NPM not found")
		return()
	endif()

	# Dependencies
	add_custom_target(${target}_depends
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
		COMMAND ${CMAKE_COMMAND} -E echo "Installing ${target} dependencies"
		COMMAND ${NPM_EXECUTABLE} install
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/package.json ${CMAKE_CURRENT_SOURCE_DIR}/lib/package-lock.json
	)

	set_target_properties(${target}_depends
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS}
		FOLDER "${IDE_FOLDER}"
	)


	# Build Bootstrap
	add_custom_target(${target}_build
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
		COMMAND ${NPM_EXECUTABLE} run build
		SOURCES ${sources}
		DEPENDS ${sources}
	)

	set_target_properties(${target}_build
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS}
		FOLDER "${IDE_FOLDER}"
	)

	# Copy Dependencies & Build Output
	add_custom_target(${target}
		COMMAND ${CMAKE_COMMAND} -E echo "Copying ${target} dependencies"
		COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_OUTPUT_DIR}/node_modules
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/lib/node_modules/ ${PROJECT_OUTPUT_DIR}/node_modules
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/build/bootstrap.js ${PROJECT_OUTPUT_DIR}/bootstrap.ts
		COMMAND ${CMAKE_COMMAND} -E echo "${target} dependencies copied from ${CMAKE_CURRENT_SOURCE_DIR}/node_modules to ${PROJECT_OUTPUT_DIR}/node_modules"
		SOURCES ${sources}
		DEPENDS ${sources} ${CMAKE_CURRENT_SOURCE_DIR}/lib/package.json ${CMAKE_CURRENT_SOURCE_DIR}/lib/package-lock.json
	)

	add_dependencies(${target}_build ${target}_depends)
	add_dependencies(${target} ${target}_build ${target}_depends)

	# Test
	add_test(NAME ${target}
		COMMAND ${NPM_EXECUTABLE} run test
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
	)

	set_property(TEST ${target}
		PROPERTY LABELS ${target}
	)

	# Install TypeScript
	install(DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}/lib/node_modules/typescript
		DESTINATION ${INSTALL_LIB}/node_modules
		COMPONENT runtime
	)
else()
	set(NodeJS_EXECUTABLE_ONLY ON)

	find_package(NodeJS)

	if(NOT NodeJS_FOUND)
		message(STATUS "NodeJS executable not found, skipping ts loader bootstrap build")
		return()
	endif()

	# Build Bootstrap (without typechecking)
	add_custom_target(${target} ALL
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
		COMMAND ${NodeJS_EXECUTABLE} compile.js
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/build/bootstrap.js ${PROJECT_OUTPUT_DIR}/bootstrap.ts
		SOURCES ${sources}
		DEPENDS ${sources}
	)

	# Guix stores typescript dependency previously to the build in the output directory
	install(DIRECTORY
		${PROJECT_OUTPUT_DIR}/node_modules/typescript
		DESTINATION ${INSTALL_LIB}/node_modules
		COMPONENT runtime
	)
endif()


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

# Install Bootstrap Library (use ts extension even if it is transpiled js to avoid conflicts with node loader bootstrap)
install(FILES
	${PROJECT_OUTPUT_DIR}/bootstrap.ts
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
