#
# Configuration for all sub-projects
#

# Generate plugin-header
configure_file(plugins/plugins.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/${META_PROJECT_NAME}/${META_PROJECT_NAME}-plugins.h)

# Export output script directory
set(LOADER_SCRIPT_PATH "${PROJECT_BINARY_DIR}/scripts/" CACHE PATH "MetaCall scripts path")

# Export output plugin directory
set(LOADER_LIBRARY_PATH "@OUTPUT_DIRECTORY_DIR@" CACHE PATH "MetaCall plugins path")

# Export output configuration directory
set(CONFIGURATION_DIR "${PROJECT_BINARY_DIR}/configurations" CACHE PATH "MetaCall configuration directory")
set(CONFIGURATION_PATH "${CONFIGURATION_DIR}/global.json" CACHE PATH "MetaCall configuration path")

# Export output serial plugin directory
set(SERIAL_LIBRARY_PATH "@OUTPUT_DIRECTORY_DIR@" CACHE PATH "MetaCall serial library path")

# Add extra environment varible
set(EXTRA_ENVIRONMENT_VARIABLES "" CACHE PATH "MetaCall extra environment variable")

#
# Configure variables
#

set(TESTS_LOADER_ENVIRONMENT_VARIABLES
	"LOADER_LIBRARY_PATH=${LOADER_LIBRARY_PATH}"
	"LOADER_SCRIPT_PATH=${LOADER_SCRIPT_PATH}"
)

set(TESTS_CONFIGURATION_ENVIRONMENT_VARIABLES
	"CONFIGURATION_PATH=${CONFIGURATION_PATH}"
)

set(TESTS_SERIAL_ENVIRONMENT_VARIABLES
	"SERIAL_LIBRARY_PATH=${SERIAL_LIBRARY_PATH}"
)

set(TESTS_ENVIRONMENT_VARIABLES
	${TESTS_LOADER_ENVIRONMENT_VARIABLES}
	${TESTS_CONFIGURATION_ENVIRONMENT_VARIABLES}
	${TESTS_SERIAL_ENVIRONMENT_VARIABLES}
	${EXTRA_ENVIRONMENT_VARIABLES}
)

#
# CMake modules
#

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
# Sub-projects
#

# Libraries
set(IDE_FOLDER "")
add_subdirectory(version)
add_subdirectory(preprocessor)
add_subdirectory(environment)
add_subdirectory(format)
add_subdirectory(log)
add_subdirectory(adt)
add_subdirectory(filesystem)
add_subdirectory(dynlink)
add_subdirectory(reflect)
add_subdirectory(serial)
add_subdirectory(configuration)
add_subdirectory(loader)
add_subdirectory(metacall)

# Tests
set(IDE_FOLDER "Tests")
add_subdirectory(tests)

# Examples
set(IDE_FOLDER "Examples")
add_subdirectory(examples)

# Plugins
set(IDE_FOLDER "Plugins")
add_subdirectory(plugins)

# Scripts
set(IDE_FOLDER "Scripts")
add_subdirectory(scripts)

# Serials
set(IDE_FOLDER "Serials")
add_subdirectory(serials)

# Distributable
set(IDE_FOLDER "Distributable")
add_subdirectory(distributable)

# Ports
if(OPTION_BUILD_PORTS)
	set(IDE_FOLDER "Ports")
	add_subdirectory(ports)
endif()

#
# Deployment
#

# Deploy generated headers
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/${META_PROJECT_NAME} DESTINATION include COMPONENT dev)

# Show deploy info
function(install_print color msg)
	install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --${color} --bold \"${msg}\")")
endfunction()

install_print("yellow" "MetaCall configuration paths (overwritables)")

install_print("green" "LOADER_SCRIPT_PATH:")
	install_print("normal" "\tDescription: Directory where scripts are located")
	install_print("normal" "\tInstall Location: N/A")
	install_print("normal" "\tDefault Location: scripts")

if(BUILD_SHARED_LIBS)
	install_print("green" "LOADER_LIBRARY_PATH:")
		install_print("normal" "\tDescription: Directory where MetaCall loader plugins are located")
		install_print("normal" "\tInstall Location: ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB}")
		install_print("normal" "\tDefault Location: .")
endif()

install_print("green" "CONFIGURATION_PATH:")
	install_print("normal" "\tDescription: Path to the main global MetaCall configuration")
	install_print("normal" "\tInstall Location: ${CMAKE_INSTALL_PREFIX}/${INSTALL_DATA}/configurations/global.json")
	install_print("normal" "\tDefault Location: configurations/global.json")

if(BUILD_SHARED_LIBS)
	install_print("green" "SERIAL_LIBRARY_PATH:")
		install_print("normal" "\tDescription: Directory where MetaCall serial plugins are located")
		install_print("normal" "\tInstall Location: ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB}")
		install_print("normal" "\tDefault Location: serials")
endif()
