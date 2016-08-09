#
# Configuration for all sub-projects
#

# Generate version-header
string(TOUPPER ${META_PROJECT_NAME} META_PROJECT_NAME_UPPER)
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/${META_PROJECT_NAME}/${META_PROJECT_NAME}-version.h)

#
# Sub-projects
#

# Libraries
set(IDE_FOLDER "")
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

# Scripts
set(IDE_FOLDER "Ports")
add_subdirectory(ports)

#
# Deployment
#

# Deploy generated headers
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/${META_PROJECT_NAME} DESTINATION include COMPONENT dev)
