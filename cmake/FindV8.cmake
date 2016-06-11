#
# V8 Google JavaScript Engine by Parra Studios
# Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Find V8 library and include paths
#
# V8_FOUND - True if V8 was found
# V8_INCLUDE_DIR - V8 headers path
# V8_LIBRARIES - List of V8 libraries

# Prevent vervosity if already included
if(V8_INCLUDE_DIR)
    set(V8_FIND_QUIETLY TRUE)
endif()

# V8 search paths
set(V8_PATHS
	${V8_HOME}
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
	)

	set(V8_PATHS_RELEASE
		${V8_DIR}/out/ia32.release
		${V8_DIR}/out/x64.release
	)
endif()

# V8 library paths
set(V8_LIBRARY_PATH_SUFFIXES lib lib64 lib.target)

# Find include path

if(MSVC OR CMAKE_BUILD_TYPE EQUAL "DEBUG")
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
set(V8_NAMES_DEBUG v8D v8_baseD v8_base.ia32D v8_base.x64D libv8_baseD v8_baseD.lib)
set(V8_NAMES_RELEASE v8 v8_base v8_base.ia32 v8_base.x64 libv8_base v8_base.lib)
set(V8_SNAPSHOT_NAMES_DEBUG v8_snapshotD libv8_snapshotD v8_snapshotD.lib)
set(V8_SNAPSHOT_NAMES_RELEASE v8_snapshot libv8_snapshot v8_snapshot.lib)
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

# Include package manager
include(FindPackageHandleStandardArgs)

if(V8_LIBRARY_DEBUG AND V8_LIBRARY_RELEASE AND V8_SNAPSHOT_LIBRARY_DEBUG AND V8_SNAPSHOT_LIBRARY_RELEASE)
	if(MSVC)
		set(V8_LIBRARY
			${V8_LIBRARY_DEBUG} ${V8_SNAPSHOT_LIBRARY_DEBUG}
			${V8_LIBRARY_RELEASE} ${V8_SNAPSHOT_LIBRARY_RELEASE}
		)

		set(V8_LIBRARIES
			optimized ${V8_LIBRARY_RELEASE} debug ${V8_LIBRARY_DEBUG}
			optimized ${V8_SNAPSHOT_LIBRARY_RELEASE} debug ${V8_SNAPSHOT_LIBRARY_DEBUG}
			optimized Winmm.lib debug Winmm.lib
		)
	else()
		if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
			set(V8_LIBRARY ${V8_LIBRARY_DEBUG} ${V8_SNAPSHOT_LIBRARY_DEBUG})
		else()
			set(V8_LIBRARY ${V8_LIBRARY_RELEASE} ${V8_SNAPSHOT_LIBRARY_RELEASE})
		endif()

		set(V8_LIBRARIES ${V8_LIBRARY})
	endif()
endif()

find_package_handle_standard_args(V8 DEFAULT_MSG V8_LIBRARY V8_INCLUDE_DIR)

mark_as_advanced(V8_LIBRARY V8_INCLUDE_DIR)

if(V8_FOUND)
	set(V8_INCLUDE_DIRS ${V8_INCLUDE_DIR})
endif()
