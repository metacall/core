# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE)
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target node_loader_bootstrap)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target} bootstrap")

#
# Create target
#

if(MSVC)
	set(LOADER_LIBRARY_PATH "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
	set(LOADER_LIBRARY_PATH "${PROJECT_BINARY_DIR}")
endif()

if(OPTION_BUILD_GUIX)
	add_custom_target(${target} ALL
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/bootstrap.js ${LOADER_LIBRARY_PATH}
	)
else()
	add_custom_target(${target} ALL
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/bootstrap.js ${LOADER_LIBRARY_PATH}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/package.json ${LOADER_LIBRARY_PATH}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/package-lock.json ${LOADER_LIBRARY_PATH}
		COMMAND npm install
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

# Library
install(FILES
	${LOADER_LIBRARY_PATH}/bootstrap.js
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)

if(NOT OPTION_BUILD_GUIX)
	# In Guix Bootstrap dependencies (aka Cherow
	# at the moment of writting) must be installed manually
	install(DIRECTORY
		${LOADER_LIBRARY_PATH}/node_modules
		DESTINATION ${INSTALL_LIB}
		COMPONENT runtime
	)
endif()
