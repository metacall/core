#
# Configuration for all sub-projects
#

# Generate plugin-header
configure_file(plugins/plugins.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/${META_PROJECT_NAME}/${META_PROJECT_NAME}-plugins.h)

# Export output script directory
set(LOADER_SCRIPT_PATH "${PROJECT_BINARY_DIR}/scripts/")

# Export output plugin directory
set(LOADER_LIBRARY_PATH "${PROJECT_BINARY_DIR}/")

#
# Sub-projects
#

# Libraries
set(IDE_FOLDER "")
add_subdirectory(version)
add_subdirectory(preprocessor)
add_subdirectory(format)
add_subdirectory(log)
add_subdirectory(adt)
add_subdirectory(filesystem)
add_subdirectory(dynlink)
add_subdirectory(reflect)
add_subdirectory(loader)
add_subdirectory(metacall)
add_subdirectory(metacallpp)

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
#set(IDE_FOLDER "Ports")
#add_subdirectory(ports)

#
# Deployment
#

# Deploy generated headers
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/${META_PROJECT_NAME} DESTINATION include COMPONENT dev)
