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

# Export output configuration plugin directory
set(CONFIGURATION_LIBRARY_PATH "@OUTPUT_DIRECTORY_DIR@" CACHE PATH "MetaCall configuration library path")

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
	"CONFIGURATION_LIBRARY_PATH=${CONFIGURATION_LIBRARY_PATH}"
)

set(TESTS_ENVIRONMENT_VARIABLES
	${TESTS_LOADER_ENVIRONMENT_VARIABLES}
	${TESTS_CONFIGURATION_ENVIRONMENT_VARIABLES}
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
add_subdirectory(configuration)
add_subdirectory(dynlink)
add_subdirectory(reflect)
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

# Ports
if(OPTION_BUILD_PORTS)
	set(IDE_FOLDER "Ports")
	add_subdirectory(ports)
endif()

# Configurations
if(OPTION_BUILD_CONFIGS)
	set(IDE_FOLDER "Configurations")
	add_subdirectory(configs)

	# Install runtime configuration
	install(DIRECTORY ${CONFIGURATION_DIR} DESTINATION ${INSTALL_DATA} COMPONENT runtime)
endif()

# Distributable
if(BUILD_DISTRIBUTABLE_LIBS)
	set(IDE_FOLDER "Distributable")
	add_subdirectory(distributable)
endif()

#
# Deployment
#

# Deploy generated headers
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/${META_PROJECT_NAME} DESTINATION include COMPONENT dev)
