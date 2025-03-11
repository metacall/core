#
#	CMake Find LibSecComp library by Parra Studios
#	CMake script to find SecComp library.
#
#	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# Find SecComp library and include paths
#
# LibSecComp_FOUND - True if SecComp library was found
# LibSecComp_INCLUDE_DIR - SecComp headers path
# LibSecComp_LIBRARY - List of SecComp libraries
# LibSecComp::LibSecComp - CMake target library

# Prevent vervosity if already included
if(LibSecComp_FOUND)
	set(LibSecComp_FIND_QUITELY TRUE)
endif()

# Try to find the library paths with pkg-config
find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(PKG_LibSecComp QUIET libseccomp libseccomp2)
endif()

# Find library includes
find_path(LibSecComp_INCLUDE_DIRS
	NAMES seccomp.h
	HINTS ${PKG_LibSecComp_INCLUDE_DIRS}
)

# Find library
set(LibSecComp_SUFFIXES
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

# Require LibSecComp as shared library
set(LibSecComp_LIBRARY_NAMES
	${CMAKE_SHARED_LIBRARY_PREFIX}seccomp${CMAKE_SHARED_LIBRARY_SUFFIX}
)

if(LibSecComp_FIND_VERSION)
	set(LibSecComp_LIBRARY_NAMES
		${LibSecComp_LIBRARY_NAMES}
		${CMAKE_SHARED_LIBRARY_PREFIX}seccomp${CMAKE_SHARED_LIBRARY_SUFFIX}.${LibSecComp_FIND_VERSION}
	)
endif()

find_library(LibSecComp_LIBRARIES
	NAMES ${LibSecComp_LIBRARY_NAMES}
	HINTS ${PKG_LibSecComp_LIBRARY_DIRS}
	PATHS /usr /usr/lib /usr/local /opt/local
	PATH_SUFFIXES lib lib64 ${LibSecComp_SUFFIXES}
)

# Declare the library and required paths
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibSecComp
	FOUND_VAR LibSecComp_FOUND
	REQUIRED_VARS LibSecComp_LIBRARIES LibSecComp_INCLUDE_DIRS
)

# Define CMake library target
if (LibSecComp_FOUND AND NOT TARGET LibSecComp::LibSecComp)
	add_library(LibSecComp::LibSecComp UNKNOWN IMPORTED)
	set_target_properties(LibSecComp::LibSecComp PROPERTIES
		IMPORTED_LOCATION "${LibSecComp_LIBRARIES}"
		INTERFACE_INCLUDE_DIRECTORIES "${LibSecComp_INCLUDE_DIRS}"
	)
endif()

mark_as_advanced(LibSecComp_LIBRARIES LibSecComp_INCLUDE_DIRS)

# Add extra properties
include(FeatureSummary)
set_package_properties(LibSecComp PROPERTIES
	URL "https://github.com/seccomp/libseccomp"
	DESCRIPTION "The libseccomp library provides an easy to use, platform independent, interface to the Linux Kernel's syscall filtering mechanism."
)
