#
#	CMake Find LuaJIT library by Parra Studios
#	CMake script to find LuaJIT library.
#
#	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# Find LuaJIT library and include paths
#
# LUAJIT_FOUND - True if LuaJIT library was found
# LUAJIT_INCLUDE_DIR - LuaJIT headers path
# LUAJIT_LIBRARY - List of LuaJIT libraries

# Prevent verbosity if already included
if(LUAJIT_FOUND)
	set(LUAJIT_FIND_QUIETLY TRUE)
endif()

include(FindPackageHandleStandardArgs)

set(LUAJIT_SUFFIXES
	x86_64-linux-gnu
	aarch64-linux-gnu
	arm-linux-gnueabi
	arm-linux-gnueabihf
	i386-linux-gnu
	mips64el-linux-gnuabi64
	mipsel-linux-gnu
	powerpc64le-linux-gnu
	s390x-linux-gnu
)

# LuaJIT library names vary by platform
set(LUAJIT_LIBRARY_NAMES
	luajit-5.1
	${CMAKE_SHARED_LIBRARY_PREFIX}luajit-5.1${CMAKE_SHARED_LIBRARY_SUFFIX}
	${CMAKE_STATIC_LIBRARY_PREFIX}luajit-5.1${CMAKE_STATIC_LIBRARY_SUFFIX}
)

find_library(LUAJIT_LIBRARY
	NAMES ${LUAJIT_LIBRARY_NAMES}
	PATHS
	/usr /usr/local /opt/luajit
	~/Library/Frameworks
	/Library/Frameworks
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	PATH_SUFFIXES lib lib64 ${LUAJIT_SUFFIXES}
)

find_path(LUAJIT_INCLUDE_DIR luajit.h
	PATHS
	/usr /usr/local /opt/luajit
	~/Library/Frameworks
	/Library/Frameworks
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	PATH_SUFFIXES include/luajit-2.1 include/luajit-2.0 include/luajit-5_1-2.1 include/luajit-5_1-2.0 include luajit
)

# Define LuaJIT cmake module
find_package_handle_standard_args(LuaJIT DEFAULT_MSG LUAJIT_LIBRARY LUAJIT_INCLUDE_DIR)

# Mark cmake module as advanced
mark_as_advanced(LUAJIT_INCLUDE_DIR LUAJIT_LIBRARY)
