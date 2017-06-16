
# Append cmake path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Ruby project utility
include(RubyProject)

#
# Sub-projects
#

add_subdirectory(hello)
add_subdirectory(second)
add_subdirectory(blog)
add_subdirectory(cache)
add_subdirectory(ducktype)
