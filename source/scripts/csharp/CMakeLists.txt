
# Append cmake path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# CSharp project utility
include(CSharpProject)

#
# Sub-projects
#

add_subdirectory(hello)
