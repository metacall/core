#
# External dependencies
#

find_package(PythonInterp)

if(NOT PYTHONINTERP_FOUND)
	message(STATUS "Python interpreter not found")
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacallpy)

# Exit here if required dependencies are not met
message(STATUS "Example ${target}")

#
# Sources
#

set(sources
	main.py
)

#
# Create executable
#

# Build executable
add_custom_target(${target}
	MACOSX_BUNDLE
	ALL
	DEPENDS ${sources}
)

# Create namespaced alias
#add_custom_target(${META_PROJECT_NAME}::${target} ALIAS ${target})

#
# Project options
#

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

#
# Include directories
#

#target_include_directories(${target}
#	PRIVATE
#	${DEFAULT_INCLUDE_DIRECTORIES}
#	${PROJECT_BINARY_DIR}/source/include
#)

#
# Libraries
#

#target_link_libraries(${target}
#	PRIVATE
#	${DEFAULT_LIBRARIES}
#	${META_PROJECT_NAME}::py_port
#)

#
# Compile definitions
#

#target_compile_definitions(${target}
#	PRIVATE
#	${DEFAULT_COMPILE_DEFINITIONS}
#)

#
# Compile options
#

#target_compile_options(${target}
#	PRIVATE
#	${DEFAULT_COMPILE_OPTIONS}
#)

#
# Linker options
#

#target_link_libraries(${target}
#	PRIVATE
#	${DEFAULT_LINKER_OPTIONS}
#)

#
# Deployment
#

# Executable
#install(TARGETS ${target}
#	RUNTIME DESTINATION ${INSTALL_BIN} COMPONENT examples
#	BUNDLE  DESTINATION ${INSTALL_BIN} COMPONENT examples
#)
