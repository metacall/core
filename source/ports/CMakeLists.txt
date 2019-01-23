#
#	MetaCall Ports by Parra Studios
#	A complete infrastructure for supporting multiple language bindings in MetaCall.
#
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
#

# Check if ports are enabled
if(NOT OPTION_BUILD_PORTS)
	return()
endif()

#
# Project configuration options
#

# Project options
option(OPTION_BUILD_PORTS_CS	"Build C# port."				ON)
option(OPTION_BUILD_PORTS_CXX	"Build C++ port."				ON)
option(OPTION_BUILD_PORTS_D		"Build D port."					OFF)
option(OPTION_BUILD_PORTS_GO	"Build Go port."				OFF)
option(OPTION_BUILD_PORTS_JAVA	"Build Java port."				OFF)
option(OPTION_BUILD_PORTS_JS	"Build JavaScript port."		OFF)
option(OPTION_BUILD_PORTS_LUA	"Build Lua port."				OFF)
option(OPTION_BUILD_PORTS_NODE	"Build NodeJS port."			ON)
option(OPTION_BUILD_PORTS_PHP	"Build PHP port."				OFF)
option(OPTION_BUILD_PORTS_PL	"Build Perl port."				OFF)
option(OPTION_BUILD_PORTS_PY	"Build Python port."			ON)
option(OPTION_BUILD_PORTS_R		"Build R port."					OFF)
option(OPTION_BUILD_PORTS_RB	"Build Ruby port."				ON)

#
# Project module names
#

set(MODULE_NAMES
	cs_port
	cxx_port
	d_port
	go_port
	java_port
	js_port
	lua_port
	node_port
	php_port
	pl_port
	py_port
	r_port
	rb_port
)

#
# External dependencies
#

# MetaCall Library
find_package(MetaCall)

if(NOT METACALL_FOUND)
	message(STATUS "MetaCall libraries not found")
	return()
endif()

include_directories(${METACALL_INCLUDE_DIRS})

#
# Port languages (Standalone)
#

add_subdirectory(cxx_port)

#
# External dependencies
#

# SWIG
find_package(SWIG)

if(NOT SWIG_FOUND)
	message(STATUS "Swig not found")
	return()
endif()

include(${SWIG_USE_FILE})

#
# Port languages (Swig)
#

add_subdirectory(cs_port)
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
