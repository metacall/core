# Check if this script is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY)
	return()
endif()

# Append cmake path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Python project utility
include(PythonProject)

#
# Sub-projects
#

add_subdirectory(example)
add_subdirectory(helloworld)
add_subdirectory(initfini)
add_subdirectory(callback)
add_subdirectory(function)
add_subdirectory(ducktype)
add_subdirectory(rsasample)
add_subdirectory(garbage)
add_subdirectory(classname)
add_subdirectory(web)
add_subdirectory(landing)
add_subdirectory(model)
add_subdirectory(pointer)
add_subdirectory(dicty)
add_subdirectory(host)
add_subdirectory(s1)
add_subdirectory(s2)
add_subdirectory(withoutfunctions)
add_subdirectory(wasm)
add_subdirectory(badimport)
add_subdirectory(watzon)
add_subdirectory(fnmesh)
