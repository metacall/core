#
# CMake Find LibGit2 Library by Parra Studios
# Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Find libgit2 library and include paths
#
# LibGit2_FOUND - True if LibGit2 was found
# LibGit2_INCLUDE_DIR - LibGit2 headers path
# LibGit2_VERSION - LibGit2 version
# LibGit2_VERSION_MAJOR - LibGit2 major version
# LibGit2_VERSION_MINOR - LibGit2 minor version
# LibGit2_VERSION_REVISION - LibGit2 patch version
# LibGit2_LIBRARY - LibGit2 shared library
# LibGit2_LIBRARY_DIR - LibGit2 shared library folder
#

# Prevent vervosity if already included
if(LibGit2_LIBRARY)
	set(LibGit2_FIND_QUIETLY TRUE)
endif()

# Include package manager
include(FindPackageHandleStandardArgs)

# Find via PkgConfig
find_package(PkgConfig QUIET)
pkg_check_modules(PKG_GIT2 QUIET libgit2)

if(NOT PKG_GIT2_FOUND)
	return()
endif()

if(NOT LibGit2_DEFINITIONS)
	set(LibGit2_DEFINITIONS ${PKG_GIT2_CFLAGS_OTHER})
endif()

if(NOT LibGit2_INCLUDE_DIR)
	find_path(LibGit2_INCLUDE_DIR
		NAMES git2.h
		HINTS ${PKG_GIT2_INCLUDE_DIRS}
	)
endif()

if(NOT LibGit2_VERSION AND LibGit2_INCLUDE_DIR)
	file(STRINGS "${LibGit2_INCLUDE_DIR}/git2/version.h" LibGit2_VERSION_MAJOR REGEX "^#define LIBGIT2_VER_MAJOR +([0-9]+)")
	string(REGEX MATCH "([0-9]+)$" LibGit2_VERSION_MAJOR "${LibGit2_VERSION_MAJOR}")

	file(STRINGS "${LibGit2_INCLUDE_DIR}/git2/version.h" LibGit2_VERSION_MINOR REGEX "^#define LIBGIT2_VER_MINOR +([0-9]+)")
	string(REGEX MATCH "([0-9]+)$" LibGit2_VERSION_MINOR "${LibGit2_VERSION_MINOR}")

	file(STRINGS "${LibGit2_INCLUDE_DIR}/git2/version.h" LibGit2_VERSION_REVISION REGEX "^#define LIBGIT2_VER_REVISION +([0-9]+)")
	string(REGEX MATCH "([0-9]+)$" LibGit2_VERSION_REVISION "${LibGit2_VERSION_REVISION}")

	set(LibGit2_VERSION "${LibGit2_VERSION_MAJOR}.${LibGit2_VERSION_MINOR}.${LibGit2_VERSION_REVISION}")
endif()

if(NOT LibGit2_LIBRARY)
	find_library(LibGit2_LIBRARY
		NAMES git2
		HINTS ${PKG_GIT2_LIBRARY_DIRS}
	)
endif()

set(LibGit2_LIBRARIES ${LibGit2_LIBRARY})
set(LibGit2_INCLUDE_DIRS ${LibGit2_INCLUDE_DIR})
get_filename_component(LibGit2_LIBRARY_DIR ${LibGit2_LIBRARY} DIRECTORY)

# Define package
find_package_handle_standard_args(LibGit2
	FOUND_VAR
		LibGit2_FOUND
	REQUIRED_VARS
		LibGit2_LIBRARY
		LibGit2_LIBRARY_DIR
		LibGit2_INCLUDE_DIR
		LibGit2_INCLUDE_DIRS
	VERSION_VAR
		LibGit2_VERSION
)

mark_as_advanced(LibGit2_LIBRARY LibGit2_LIBRARY_DIR LibGit2_INCLUDE_DIR)
