#
#	CMake Find Tiny C Compiler library by Parra Studios
#	CMake script to find TCC library.
#
#	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# Find TCC library and include paths
#
# LIBTCC_FOUND - True if TCC library was found
# LIBTCC_INCLUDE_DIR - TCC headers path
# LIBTCC_LIBRARY - List of TCC libraries

# Prevent vervosity if already included
if(LIBTCC_FOUND)
	set(LIBTCC_FIND_QUITELY TRUE)
endif()

include(FindPackageHandleStandardArgs)

set(LIBTCC_SUFFIXES
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

# Require TCC as shared library
set(LIBTCC_LIBRARY_NAMES
	${CMAKE_SHARED_LIBRARY_PREFIX}tcc${CMAKE_SHARED_LIBRARY_SUFFIX}
	${CMAKE_SHARED_LIBRARY_PREFIX}libtcc${CMAKE_SHARED_LIBRARY_SUFFIX}
)

find_library(LIBTCC_LIBRARY
	NAMES ${LIBTCC_LIBRARY_NAMES}
	PATHS /usr /usr/lib /usr/local /opt/local
	PATH_SUFFIXES lib lib64 ${LIBTCC_SUFFIXES}
)

find_path(LIBTCC_INCLUDE_DIR libtcc.h
	PATHS /usr /usr/include /usr/local /opt/local
	PATH_SUFFIXES ${LIBTCC_SUFFIXES}
)

# Define TCC cmake module
find_package_handle_standard_args(LibTCC DEFAULT_MSG LIBTCC_LIBRARY LIBTCC_INCLUDE_DIR)

# Mark cmake module as advanced
mark_as_advanced(LIBTCC_INCLUDE_DIR LIBTCC_LIBRARY)
