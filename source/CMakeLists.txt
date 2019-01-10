#
#	MetaCall Ports by Parra Studios
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	A complete infrastructure for supporting multiple language bindings in MetaCall.
#

#
# External dependencies
#

# SWIG
find_package(SWIG REQUIRED)

if(NOT SWIG_FOUND)
	message(STATUS "Swig not found")
	return()
endif()

include(${SWIG_USE_FILE})

# MetaCall Library
find_package(MetaCall REQUIRED)

if(NOT METACALL_FOUND)
	message(STATUS "MetaCall libraries not found")
	return()
endif()

include_directories(${METACALL_INCLUDE_DIRS})

#
# Port languages
#

add_subdirectory(cs_port)
add_subdirectory(cxx_port)
add_subdirectory(d_port)
add_subdirectory(go_port)
add_subdirectory(java_port)
add_subdirectory(js_port)
add_subdirectory(lua_port)
#add_subdirectory(node_port)
add_subdirectory(php_port)
add_subdirectory(pl_port)
add_subdirectory(py_port)
add_subdirectory(rb_port)
add_subdirectory(r_port)
