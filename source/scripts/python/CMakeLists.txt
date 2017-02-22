
# Append cmake path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Python project utility
include(PythonProject)

#
# Sub-projects
#

add_subdirectory(example)
add_subdirectory(frontend)
add_subdirectory(initfini)
