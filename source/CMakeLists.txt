#
# Configuration for all sub-projects
#

# Export output script directory
set(LOADER_SCRIPT_PATH "${PROJECT_BINARY_DIR}/scripts" CACHE PATH "MetaCall scripts path")

# Export output loader plugin directory
set(LOADER_LIBRARY_PATH "@OUTPUT_DIRECTORY_DIR@" CACHE PATH "MetaCall loaders path")

# Export output configuration directory
set(CONFIGURATION_DIR "${PROJECT_BINARY_DIR}/configurations" CACHE PATH "MetaCall configuration directory")
set(CONFIGURATION_PATH "${CONFIGURATION_DIR}/global.json" CACHE PATH "MetaCall configuration path")

# Export output serial plugin directory
set(SERIAL_LIBRARY_PATH "@OUTPUT_DIRECTORY_DIR@" CACHE PATH "MetaCall serial library path")

# Export output detour plugin directory
set(DETOUR_LIBRARY_PATH "@OUTPUT_DIRECTORY_DIR@" CACHE PATH "MetaCall detour library path")

# Export output port directory
set(PORT_LIBRARY_PATH "@OUTPUT_DIRECTORY_DIR@" CACHE PATH "MetaCall port library path")

# Add extra environment varible
set(EXTRA_ENVIRONMENT_VARIABLES "" CACHE PATH "MetaCall extra environment variable")

#
# Configure templates
#

string(TOLOWER ${META_PROJECT_NAME} META_PROJECT_NAME_LOWER)

# Generate loaders plugin header
configure_file(loaders/loaders.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/${META_PROJECT_NAME_LOWER}/${META_PROJECT_NAME_LOWER}_loaders.h)

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

set(TESTS_DETOUR_ENVIRONMENT_VARIABLES
	"DETOUR_LIBRARY_PATH=${DETOUR_LIBRARY_PATH}"
)

set(TESTS_PORT_ENVIRONMENT_VARIABLES
	"PORT_LIBRARY_PATH=${PORT_LIBRARY_PATH}"
)

set(TESTS_ENVIRONMENT_VARIABLES
	${TESTS_LOADER_ENVIRONMENT_VARIABLES}
	${TESTS_CONFIGURATION_ENVIRONMENT_VARIABLES}
	${TESTS_SERIAL_ENVIRONMENT_VARIABLES}
	${TESTS_DETOUR_ENVIRONMENT_VARIABLES}
	${TESTS_PORT_ENVIRONMENT_VARIABLES}
	${TESTS_SANITIZER_ENVIRONMENT_VARIABLES}
	${EXTRA_ENVIRONMENT_VARIABLES}
)

#
# CMake modules
#

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
# Default definitions
#

add_definitions(
	-DCONFIGURATION_INSTALL_PATH=\"${CMAKE_INSTALL_PREFIX}/${INSTALL_DATA}/configurations/global.json\"
	-DSERIAL_LIBRARY_INSTALL_PATH=\"${CMAKE_INSTALL_PREFIX}/${INSTALL_SHARED}\"
	-DLOADER_LIBRARY_INSTALL_PATH=\"${CMAKE_INSTALL_PREFIX}/${INSTALL_SHARED}\"
	-DDETOUR_LIBRARY_INSTALL_PATH=\"${CMAKE_INSTALL_PREFIX}/${INSTALL_SHARED}\"
	-DPORT_LIBRARY_INSTALL_PATH=\"${CMAKE_INSTALL_PREFIX}/${INSTALL_SHARED}\"
)

#
# Sub-projects
#

# Libraries
set(IDE_FOLDER "")
add_subdirectory(version)
add_subdirectory(preprocessor)
add_subdirectory(environment)
add_subdirectory(format)
add_subdirectory(backtrace)
add_subdirectory(log)
add_subdirectory(memory)
add_subdirectory(portability)
add_subdirectory(threading)
add_subdirectory(adt)
add_subdirectory(filesystem)
add_subdirectory(dynlink)
add_subdirectory(plugin)
add_subdirectory(detour)
add_subdirectory(reflect)
add_subdirectory(serial)
add_subdirectory(configuration)
add_subdirectory(loader)
add_subdirectory(metacall)

# Loaders
set(IDE_FOLDER "Loaders")
add_subdirectory(loaders)

# Serials
set(IDE_FOLDER "Serials")
add_subdirectory(serials)

# Serials
set(IDE_FOLDER "Detours")
add_subdirectory(detours)

# Ports
set(IDE_FOLDER "Ports")
add_subdirectory(ports)

# Scripts
set(IDE_FOLDER "Scripts")
add_subdirectory(scripts)

# Tests
set(IDE_FOLDER "Tests")
add_subdirectory(tests)

# Benchmarks
set(IDE_FOLDER "Benchmarks")
add_subdirectory(benchmarks)

# CLIs
set(IDE_FOLDER "CLIs")
add_subdirectory(cli)

# Examples
set(IDE_FOLDER "Examples")
add_subdirectory(examples)

#
# Deployment
#

string(TOLOWER ${META_PROJECT_NAME} META_PROJECT_NAME_LOWER)

# Deploy generated headers
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/${META_PROJECT_NAME_LOWER} DESTINATION include COMPONENT dev)

# Show deploy info
function(install_print color msg)
	install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --${color} --bold \"${msg}\")")
endfunction()

install_print("yellow" "MetaCall configuration paths (overwritables)")

install_print("green" "LOADER_SCRIPT_PATH:")
	install_print("normal" "\tDescription: Directory where scripts are located")
	install_print("normal" "\tInstall Location: N/A")
	install_print("normal" "\tDefault Location: scripts")

install_print("green" "CONFIGURATION_PATH:")
	install_print("normal" "\tDescription: Path to the main global MetaCall configuration")
	install_print("normal" "\tInstall Location: ${CMAKE_INSTALL_PREFIX}/${INSTALL_DATA}/configurations/global.json")
	install_print("normal" "\tDefault Location: configurations/global.json")

if(BUILD_SHARED_LIBS)
	install_print("green" "LOADER_LIBRARY_PATH:")
		install_print("normal" "\tDescription: Directory where MetaCall loader plugins are located")
		install_print("normal" "\tInstall Location: ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB}")
		install_print("normal" "\tDefault Location: .")

	install_print("green" "SERIAL_LIBRARY_PATH:")
		install_print("normal" "\tDescription: Directory where MetaCall serial plugins are located")
		install_print("normal" "\tInstall Location: ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB}")
		install_print("normal" "\tDefault Location: serials")

	install_print("green" "DETOUR_LIBRARY_PATH:")
		install_print("normal" "\tDescription: Directory where MetaCall detour plugins are located")
		install_print("normal" "\tInstall Location: ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB}")
		install_print("normal" "\tDefault Location: detours")
endif()
