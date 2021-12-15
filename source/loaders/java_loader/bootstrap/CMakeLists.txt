# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_JAVA)
	return()
endif()

#
# External dependencies
#

find_package(Java)

if(NOT Java_JAVAC_EXECUTABLE)
	message(STATUS "Java Compiler not found")
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target java_loader_bootstrap)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target} bootstrap")

#
# Create target
#

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(sources
	${source_path}/bootstrap.java
)

# Group source files
set(source_group "Source Files")
source_group_by_path(${source_path}  "\\\\.java$"
	${source_group} ${sources})

add_custom_target(${target}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E  make_directory ${PROJECT_OUTPUT_DIR}
	COMMAND ${Java_JAVAC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/lib/bootstrap.java -d ${PROJECT_OUTPUT_DIR}
	SOURCES ${sources}
	DEPENDS ${sources}
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
	${BOOTSTRAP_OUTPUT_PATH}/bootstrap.class
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
