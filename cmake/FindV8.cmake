#
#	CMake Find V8 Google JavaScript Engine by Parra Studios
#	CMake script to find V8 JavaScript Engine.
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

# Find V8 library and include paths
#
# V8_FOUND - True if V8 was found
# V8_INCLUDE_DIR - V8 headers path
# V8_LIBRARIES - List of V8 libraries
# V8_VERSION - V8 version
# V8_VERSION_MAJOR - V8 major version
# V8_VERSION_MINOR - V8 minor version
# V8_VERSION_PATCH - V8 patch version
# V8_VERSION_TWEAK - V8 patch version
# V8_VERSION_HEX - V8 version in hexadecimal format
# V8_EXECUTABLE - V8 shell

# Prevent vervosity if already included
if(V8_INCLUDE_DIR)
	set(V8_FIND_QUIETLY TRUE)
endif()

# Debug flag
set(_V8_CMAKE_DEBUG TRUE)

# Include package manager
include(FindPackageHandleStandardArgs)

# V8 search paths
set(V8_PATHS
	${V8_HOME}
	${V8_ROOT}
	$ENV{ProgramFiles}/v8
	$ENV{SystemDrive}/v8
	$ENV{V8_HOME}
	$ENV{EXTERNLIBS}/v8
	${V8_DIR}
	$ENV{V8_DIR}
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	/usr/freeware
)

# V8 platform dependant paths
if(NOT UNIX)
	set(V8_PATHS_DEBUG
		${V8_DIR}/build/Debug
	)

	set(V8_PATHS_RELEASE
		${V8_DIR}/build/Release
	)
else()
	set(V8_PATHS_DEBUG
		${V8_DIR}/out/ia32.debug
		${V8_DIR}/out/x64.debug
		${V8_DIR}/out/native
	)

	set(V8_PATHS_RELEASE
		${V8_DIR}/out/ia32.release
		${V8_DIR}/out/x64.release
		${V8_DIR}/out/native
	)
endif()

# V8 library paths
set(V8_LIBRARY_PATH_SUFFIXES lib lib64 lib.target)

# Find include path
if(MSVC OR CMAKE_BUILD_TYPE EQUAL "Debug")
	set(V8_HEADERS v8.h v8-debug.h v8-profiler.h v8stdint.h)
else()
	set(V8_HEADERS v8.h v8stdint.h)
endif()

find_path(V8_INCLUDE_DIR ${V8_HEADERS}
	PATHS ${V8_PATHS}
	PATH_SUFFIXES include
	DOC "Google V8 JavaScript Engine Headers"
)

# Define library names
set(V8_NAMES_DEBUG v8D v8_baseD v8_base.ia32D v8_base.x64D libv8_baseD v8_baseD.lib v8_libbaseD v8_libbase.ia32D v8_libbase.x64D libv8_libbaseD v8_libbaseD.lib)
set(V8_NAMES_RELEASE v8 v8_base v8_base.ia32 v8_base.x64 libv8_base v8_base.lib v8_libbase v8_libbase.ia32 v8_libbase.x64 libv8_libbase v8_libbase.lib)
set(V8_PLATFORM_NAMES_DEBUG v8_libplatformD v8_libplatformD.a v8_libplatformD.lib)
set(V8_PLATFORM_NAMES_RELEASE v8_libplatform v8_libplatform.a v8_libplatform.lib)
set(V8_SNAPSHOT_NAMES_DEBUG v8_snapshotD libv8_snapshotD v8_snapshotD.lib)
set(V8_SNAPSHOT_NAMES_RELEASE v8_snapshot libv8_snapshot v8_snapshot.lib)
set(V8_ICU_NAMES_DEBUG icudataD icudataD.a icudataD.lib)
set(V8_ICU_NAMES_RELEASE icudata icudata.a icudata.lib)
set(V8_ICUUC_NAMES_DEBUG icuucD libicuucD)
set(V8_ICUUC_NAMES_RELEASE icuuc libicuuc)
set(V8_ICUI18N_NAMES_DEBUG icui18nD libicui18nD)
set(V8_ICUI18N_NAMES_RELEASE icui18n libicui18n)

# Find V8 base library debug
find_library(V8_LIBRARY_DEBUG
	NAMES ${V8_NAMES_DEBUG}
	PATHS ${V8_PATHS} ${V8_PATHS_DEBUG}
	PATH_SUFFIXES ${V8_LIBRARY_PATH_SUFFIXES}
	DOC "Google V8 JavaScript Engine Library (Debug)"
)

# Find V8 base library release
find_library(V8_LIBRARY_RELEASE
	NAMES ${V8_NAMES_RELEASE}
	PATHS ${V8_PATHS} ${V8_PATHS_RELEASE}
	PATH_SUFFIXES ${V8_LIBRARY_PATH_SUFFIXES}
	DOC "Google V8 JavaScript Engine Library (Release)"
)

# Find V8 platform library debug
find_library(V8_PLATFORM_LIBRARY_DEBUG
	NAMES ${V8_PLATFORM_NAMES_DEBUG}
	PATHS ${V8_PATHS} ${V8_PATHS_DEBUG}
	PATH_SUFFIXES ${V8_LIBRARY_PATH_SUFFIXES}
	DOC "Google V8 JavaScript Engine Library Platform (Debug)"
)

# Find V8 platform library release
find_library(V8_PLATFORM_LIBRARY_RELEASE
	NAMES ${V8_PLATFORM_NAMES_RELEASE}
	PATHS ${V8_PATHS} ${V8_PATHS_RELEASE}
	PATH_SUFFIXES ${V8_LIBRARY_PATH_SUFFIXES}
	DOC "Google V8 JavaScript Engine Library Platform (Release)"
)

# Find V8 snapshot library debug
find_library(V8_SNAPSHOT_LIBRARY_DEBUG
	NAMES ${V8_SNAPSHOT_NAMES_DEBUG}
	PATHS ${V8_PATHS} ${V8_PATHS_DEBUG}
	PATH_SUFFIXES ${V8_LIBRARY_PATH_SUFFIXES}
	DOC "Google V8 JavaScript Engine Library Snapshot (Debug)"
)

# Find V8 snapshot library release
find_library(V8_SNAPSHOT_LIBRARY_RELEASE
	NAMES ${V8_SNAPSHOT_NAMES_RELEASE}
	PATHS ${V8_PATHS} ${V8_PATHS_RELEASE}
	PATH_SUFFIXES ${V8_LIBRARY_PATH_SUFFIXES}
	DOC "Google V8 JavaScript Engine Library Snapshot (Release)"
)

# Find V8 icu library debug
find_library(V8_ICU_LIBRARY_DEBUG
	NAMES ${V8_ICU_NAMES_DEBUG}
	PATHS ${V8_PATHS} ${V8_PATHS_DEBUG}
	PATH_SUFFIXES ${V8_LIBRARY_PATH_SUFFIXES}
	DOC "Google V8 JavaScript Engine Library ICU (Debug)"
)

# Find V8 icu library release
find_library(V8_ICU_LIBRARY_RELEASE
	NAMES ${V8_ICU_NAMES_RELEASE}
	PATHS ${V8_PATHS} ${V8_PATHS_RELEASE}
	PATH_SUFFIXES ${V8_LIBRARY_PATH_SUFFIXES}
	DOC "Google V8 JavaScript Engine Library ICU (Release)"
)

# Find V8 icuuc library debug
find_library(V8_ICUUC_LIBRARY_DEBUG
	NAMES ${V8_ICUUC_NAMES_DEBUG}
	PATHS ${V8_PATHS} ${V8_PATHS_DEBUG}
	PATH_SUFFIXES ${V8_LIBRARY_PATH_SUFFIXES}
	DOC "Google V8 JavaScript Engine Library ICUUC (Debug)"
)

# Find V8 icuuc library release
find_library(V8_ICUUC_LIBRARY_RELEASE
	NAMES ${V8_ICUUC_NAMES_RELEASE}
	PATHS ${V8_PATHS} ${V8_PATHS_RELEASE}
	PATH_SUFFIXES ${V8_LIBRARY_PATH_SUFFIXES}
	DOC "Google V8 JavaScript Engine Library ICUUC (Release)"
)

# Find V8 icui18n library debug
find_library(V8_ICUI18N_LIBRARY_DEBUG
	NAMES ${V8_ICUI18N_NAMES_DEBUG}
	PATHS ${V8_PATHS} ${V8_PATHS_DEBUG}
	PATH_SUFFIXES ${V8_LIBRARY_PATH_SUFFIXES}
	DOC "Google V8 JavaScript Engine Library ICUI18N (Debug)"
)

# Find V8 icui18n library release
find_library(V8_ICUI18N_LIBRARY_RELEASE
	NAMES ${V8_ICUI18N_NAMES_RELEASE}
	PATHS ${V8_PATHS} ${V8_PATHS_RELEASE}
	PATH_SUFFIXES ${V8_LIBRARY_PATH_SUFFIXES}
	DOC "Google V8 JavaScript Engine Library ICUI18N (Release)"
)

# Base build with snapshot
if(MSVC)
	if(V8_LIBRARY_DEBUG AND V8_LIBRARY_RELEASE AND V8_SNAPSHOT_LIBRARY_DEBUG AND V8_SNAPSHOT_LIBRARY_RELEASE)
		set(V8_LIBRARY
			${V8_LIBRARY_DEBUG} ${V8_PLATFORM_LIBRARY_DEBUG} ${V8_SNAPSHOT_LIBRARY_DEBUG} # ${V8_ICU_LIBRARY_DEBUG}
			${V8_LIBRARY_RELEASE} ${V8_PLATFORM_LIBRARY_RELEASE} ${V8_SNAPSHOT_LIBRARY_RELEASE} # ${V8_ICU_LIBRARY_RELEASE}
		)

		set(V8_LIBRARIES
			optimized ${V8_LIBRARY_RELEASE} debug ${V8_LIBRARY_DEBUG}
			optimized ${V8_PLATFORM_LIBRARY_RELEASE} debug ${V8_PLATFORM_LIBRARY_DEBUG}
#			optimized ${V8_ICU_LIBRARY_RELEASE} debug ${V8_ICU_LIBRARY_DEBUG}
			optimized ${V8_SNAPSHOT_LIBRARY_RELEASE} debug ${V8_SNAPSHOT_LIBRARY_DEBUG}
			optimized Winmm.lib debug Winmm.lib
		)
	endif()
else()
	if(CMAKE_BUILD_TYPE EQUAL "Debug")
		if(V8_LIBRARY_DEBUG AND V8_SNAPSHOT_LIBRARY_DEBUG)
			set(V8_LIBRARY ${V8_LIBRARY_DEBUG} ${V8_PLATFORM_LIBRARY_DEBUG} ${V8_SNAPSHOT_LIBRARY_DEBUG}) # ${V8_ICU_LIBRARY_DEBUG})
		endif()
	else()
		if(V8_LIBRARY_RELEASE AND V8_SNAPSHOT_LIBRARY_RELEASE)
			set(V8_LIBRARY ${V8_LIBRARY_RELEASE} ${V8_PLATFORM_LIBRARY_RELEASE} ${V8_SNAPSHOT_LIBRARY_RELEASE}) # ${V8_ICU_LIBRARY_RELEASE})
		endif()
	endif()

	set(V8_LIBRARIES ${V8_LIBRARY})
endif()

# Set version libraries
set(V8_LIBRARIES_DEPENDS)

if (V8_LIBRARY_DEBUG)
	set(V8_LIBRARIES_DEPENDS
		${V8_LIBRARIES_DEPENDS}
		${V8_LIBRARY_DEBUG}.${V8_FIND_VERSION}
	)
endif()

if (V8_LIBRARY_RELEASE)
	set(V8_LIBRARIES_DEPENDS
		${V8_LIBRARIES_DEPENDS}
		${V8_LIBRARY_RELEASE}.${V8_FIND_VERSION}
	)
endif()

if(V8_ICU_LIBRARY_DEBUG)
	set(V8_LIBRARIES_DEPENDS
		${V8_LIBRARIES_DEPENDS}
		${V8_ICU_LIBRARY_DEBUG}.${ICU_FIND_VERSION}
	)
endif()

if(V8_ICU_LIBRARY_RELEASE)
	set(V8_LIBRARIES_DEPENDS
		${V8_LIBRARIES_DEPENDS}
		${V8_ICU_LIBRARY_RELEASE}.${ICU_FIND_VERSION}
	)
endif()

if(V8_ICUUC_LIBRARY_DEBUG)
	set(V8_LIBRARIES_DEPENDS
		${V8_LIBRARIES_DEPENDS}
		${V8_ICUUC_LIBRARY_DEBUG}.${ICU_FIND_VERSION}
	)
endif()

if(V8_ICUUC_LIBRARY_RELEASE)
	set(V8_LIBRARIES_DEPENDS
		${V8_LIBRARIES_DEPENDS}
		${V8_ICUUC_LIBRARY_RELEASE}.${ICU_FIND_VERSION}
	)
endif()

if(V8_ICUI18N_LIBRARY_DEBUG)
	set(V8_LIBRARIES_DEPENDS
		${V8_LIBRARIES_DEPENDS}
		${V8_ICUI18N_LIBRARY_DEBUG}.${ICU_FIND_VERSION}
	)
endif()

if(V8_ICUI18N_LIBRARY_RELEASE)
	set(V8_LIBRARIES_DEPENDS
		${V8_LIBRARIES_DEPENDS}
		${V8_ICUI18N_LIBRARY_RELEASE}.${ICU_FIND_VERSION}
	)
endif()

find_package_handle_standard_args(V8 DEFAULT_MSG V8_LIBRARY V8_INCLUDE_DIR)

# Base build
if(NOT V8_FOUND)
	if(MSVC)
		if(V8_LIBRARY_DEBUG AND V8_LIBRARY_RELEASE)
			set(V8_LIBRARY
				${V8_LIBRARY_DEBUG} ${V8_LIBRARY_RELEASE}
				${V8_PLATFORM_LIBRARY_DEBUG} ${V8_PLATFORM_LIBRARY_RELEASE}
#				${V8_ICU_LIBRARY_DEBUG} ${V8_ICU_LIBRARY_RELEASE}
			)

			set(V8_LIBRARIES
				optimized ${V8_LIBRARY_RELEASE} debug ${V8_LIBRARY_DEBUG}
				optimized ${V8_PLATFORM_LIBRARY_RELEASE} debug ${V8_PLATFORM_LIBRARY_DEBUG}
#				optimized ${V8_ICU_LIBRARY_RELEASE} debug ${V8_ICU_LIBRARY_DEBUG}
				optimized Winmm.lib debug Winmm.lib
			)
		endif()
	else()
		if(CMAKE_BUILD_TYPE EQUAL "Debug")
			if(V8_LIBRARY_DEBUG)
				set(V8_LIBRARY ${V8_LIBRARY_DEBUG} ${V8_PLATFORM_LIBRARY_DEBUG}) # ${V8_ICU_LIBRARY_DEBUG})
			endif()
		else()
			if(V8_LIBRARY_RELEASE)
				set(V8_LIBRARY ${V8_LIBRARY_RELEASE} ${V8_PLATFORM_LIBRARY_RELEASE}) # ${V8_ICU_LIBRARY_RELEASE})
			endif()
		endif()

		set(V8_LIBRARIES ${V8_LIBRARY})
	endif()

	find_package_handle_standard_args(V8 DEFAULT_MSG V8_LIBRARY V8_INCLUDE_DIR)
endif()

# Minimal build
if(NOT V8_FOUND)
	if(MSVC)
		if(V8_LIBRARY_RELEASE)
			set(V8_LIBRARY ${V8_LIBRARY_RELEASE} ${V8_PLATFORM_LIBRARY_RELEASE}) # ${V8_ICU_LIBRARY_RELEASE})

			set(V8_LIBRARIES
				optimized ${V8_LIBRARY_RELEASE} debug ${V8_LIBRARY_RELEASE}
				optimized ${V8_PLATFORM_LIBRARY_RELEASE} debug ${V8_PLATFORM_LIBRARY_RELEASE}
#				optimized ${V8_ICU_LIBRARY_RELEASE} debug ${V8_ICU_LIBRARY_RELEASE}
				optimized Winmm.lib debug Winmm.lib
			)
		endif()
	else()
		if(V8_LIBRARY_RELEASE)
			set(V8_LIBRARY ${V8_LIBRARY_RELEASE} ${V8_PLATFORM_LIBRARY_RELEASE}) # ${V8_ICU_LIBRARY_RELEASE})
		endif()

		set(V8_LIBRARIES ${V8_LIBRARY})
	endif()

	find_package_handle_standard_args(V8 DEFAULT_MSG V8_LIBRARY V8_INCLUDE_DIR)
endif()

# Detect V8 version
if(V8_FOUND AND V8_INCLUDE_DIR)
	file(READ ${V8_INCLUDE_DIR}/v8-version.h V8_VERSION_FILE)

	string(REGEX MATCH "#define V8_MAJOR_VERSION ([0-9]+)" V8_VERSION_MAJOR_DEF ${V8_VERSION_FILE})
	string(REGEX MATCH "([0-9]+)$" V8_VERSION_MAJOR ${V8_VERSION_MAJOR_DEF})

	string(REGEX MATCH "#define V8_MINOR_VERSION ([0-9]+)" V8_VERSION_MINOR_DEF ${V8_VERSION_FILE})
	string(REGEX MATCH "([0-9]+)$" V8_VERSION_MINOR ${V8_VERSION_MINOR_DEF})

	string(REGEX MATCH "#define V8_BUILD_NUMBER ([0-9]+)" V8_VERSION_PATCH_DEF ${V8_VERSION_FILE})
	string(REGEX MATCH "([0-9]+)$" V8_VERSION_PATCH ${V8_VERSION_PATCH_DEF})

	string(REGEX MATCH "#define V8_PATCH_LEVEL ([0-9]+)" V8_VERSION_TWEAK_DEF ${V8_VERSION_FILE})
	string(REGEX MATCH "([0-9]+)$" V8_VERSION_TWEAK ${V8_VERSION_TWEAK_DEF})

	set(V8_VERSION "${V8_VERSION_MAJOR}.${V8_VERSION_MINOR}.${V8_VERSION_PATCH}.${V8_VERSION_TWEAK}")

	set(V8_VERSION_HEX 0x0${V8_VERSION_MAJOR}${V8_VERSION_MINOR}${V8_VERSION_PATCH}${V8_VERSION_TWEAK})
	string(LENGTH "${V8_VERSION_HEX}" V8_VERSION_HEX_LENGTH)

	while(V8_VERSION_HEX_LENGTH LESS 8)

		set(V8_VERSION_HEX "${V8_VERSION_HEX}0")
		string(LENGTH "${V8_VERSION_HEX}" V8_VERSION_HEX_LENGTH)

	endwhile()
endif()

# Detect V8 shell
set(V8_EXECUTABLE_NAMES d8)

find_program(V8_EXECUTABLE
	NAMES ${V8_EXECUTABLE_NAMES}
)

mark_as_advanced(V8_LIBRARY V8_INCLUDE_DIR)

if(V8_FOUND)
	set(V8_INCLUDE_DIRS ${V8_INCLUDE_DIR})
endif()

if(_V8_CMAKE_DEBUG)
	message(STATUS "V8_INCLUDE_DIR: ${V8_INCLUDE_DIR}")
	message(STATUS "V8_LIBRARIES: ${V8_LIBRARIES}")
	message(STATUS "V8_LIBRARIES_DEPENDS: ${V8_LIBRARIES_DEPENDS}")
	message(STATUS "V8_VERSION: ${V8_VERSION}")
	message(STATUS "V8_VERSION_HEX: ${V8_VERSION_HEX}")
	message(STATUS "V8_EXECUTABLE: ${V8_EXECUTABLE}")
endif()
