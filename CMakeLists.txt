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

# Check if port is enabled
if(NOT OPTION_BUILD_PORTS)
	return()
endif()

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
# Source
#

add_subdirectory(source)
