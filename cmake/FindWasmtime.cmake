#
# CMake Find Wasmtime WebAssembly Runtime by Parra Studios
# Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Find Wasmtime library and include paths
#
# Wasmtime_LIBRARY - Wasmtime shared library
# Wasmtime_INCLUDE_DIR - Wasmtime include directory

option(Wasmtime_CMAKE_DEBUG "Show full output of the Wasmtime related commands for debugging." OFF)

include(Portability)

set(Wasmtime_VERSION ${Wasmtime_FIND_VERSION})

# See https://docs.wasmtime.dev/contributing-building.html#building-the-wasmtime-c-api
if(PROJECT_OS_LINUX)
	set(Wasmtime_LIBRARY_NAME libwasmtime.so)
	set(Wasmtime_PLATFORM "linux")
	set(Wasmtime_ARCHIVE_EXTENSION "tar.xz")
elseif(PROJECT_OS_FAMILY STREQUAL "macos")
	set(Wasmtime_LIBRARY_NAME libwasmtime.dylib)
	set(Wasmtime_PLATFORM "macos")
	set(Wasmtime_ARCHIVE_EXTENSION "tar.xz")
elseif(PROJECT_OS_WIN)
	set(Wasmtime_LIBRARY_NAME wasmtime.lib)
	set(Wasmtime_PLATFORM "windows")
	set(Wasmtime_ARCHIVE_EXTENSION "zip")
elseif(PROJECT_OS_MINGW)
	set(Wasmtime_LIBRARY_NAME wasmtime.a)
	set(Wasmtime_PLATFORM "mingw")
	set(Wasmtime_ARCHIVE_EXTENSION "zip")
else()
	message(FATAL_ERROR "Could not determine target platform or target platform is not supported")
endif()

if(PROJECT_ARCH_AMD64)
	set(Wasmtime_ARCH "x86_64")
elseif(PROJECT_ARCH_AARCH64)
	set(Wasmtime_ARCH "aarch64")
else()
	set(Wasmtime_ARCH "x86_64")
	message(WARNING "Could not determine target architecture, assuming x86_64")
endif()

message(DEBUG "Set target architecture to ${Wasmtime_ARCH}")

set(Wasmtime_LOCAL_PATH "${CMAKE_BINARY_DIR}/wasmtime")
set(Wasmtime_DOWNLOAD_DIR_NAME "wasmtime-v${Wasmtime_VERSION}-${Wasmtime_ARCH}-${Wasmtime_PLATFORM}-c-api")
set(Wasmtime_API_PATH "${Wasmtime_LOCAL_PATH}/${Wasmtime_DOWNLOAD_DIR_NAME}")
set(Wasmtime_DOWNLOAD_LIBRARY_PATH "${Wasmtime_API_PATH}/lib")
set(Wasmtime_DOWNLOAD_INCLUDE_DIR "${Wasmtime_API_PATH}/include")

if(NOT Wasmtime_LIBRARY)
	set(Wasmtime_DEFAULT_LIBRARY_PATHS
		/usr/lib
	)
	find_library(Wasmtime_LIBRARY
		NAMES ${Wasmtime_LIBRARY_NAME}
		DOC "Wasmtime C API library"
	)

	if(Wasmtime_LIBRARY)
		if(PROJECT_OS_WIN)
			string(REGEX REPLACE "[.]lib$" ".dll" Wasmtime_LIBRARY_DLL ${Wasmtime_LIBRARY})
		elseif(PROJECT_OS_MINGW)
			string(REGEX REPLACE "[.]a$" ".dll" Wasmtime_LIBRARY_DLL ${Wasmtime_LIBRARY})
		endif()
	endif()
endif()

if(NOT Wasmtime_INCLUDE_DIR)
	set(Wasmtime_DEFAULT_INCLUDE_PATHS
		/usr/include
	)
	find_path(Wasmtime_INCLUDE_DIR
		NAMES wasm.h wasmtime.h
		DOC "Wasmtime C API headers"
	)
endif()

if(NOT Wasmtime_LIBRARY OR NOT Wasmtime_INCLUDE_DIR)
	message(STATUS "Wasmtime C API library or headers not found, downloading from archive")

	if(Wasmtime_ARCH STREQUAL "x86_64" AND PROJECT_ARCH_32BIT)
		# We assumed target architecture was x86_64, but it is 32-bit, so the
		# assumption is definitely invalid.
		message(FATAL_ERROR "No downloads available for target architecture, please install Wasmtime manually")
	endif()

	set(Wasmtime_DOWNLOAD_ARCHIVE_NAME "${Wasmtime_DOWNLOAD_DIR_NAME}.${Wasmtime_ARCHIVE_EXTENSION}")
	set(Wasmtime_DOWNLOAD_URL
		"https://github.com/bytecodealliance/wasmtime/releases/download/v${Wasmtime_VERSION}/${Wasmtime_DOWNLOAD_ARCHIVE_NAME}")
	set(Wasmtime_DOWNLOAD_DEST "${Wasmtime_LOCAL_PATH}/${Wasmtime_DOWNLOAD_ARCHIVE_NAME}")

	file(DOWNLOAD ${Wasmtime_DOWNLOAD_URL} ${Wasmtime_DOWNLOAD_DEST}
		STATUS DOWNLOAD_STATUS
		SHOW_PROGRESS)

	list(GET DOWNLOAD_STATUS 0 STATUS_CODE)

	if(NOT STATUS_CODE EQUAL 0)
		list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
		message(FATAL_ERROR "Failed to download Wasmtime C API: ${ERROR_MESSAGE}")
	endif()

	file(ARCHIVE_EXTRACT
		INPUT ${Wasmtime_DOWNLOAD_DEST}
		DESTINATION ${Wasmtime_LOCAL_PATH}
	)

	if(NOT Wasmtime_LIBRARY)
		find_library(Wasmtime_LIBRARY
			NAMES ${Wasmtime_LIBRARY_NAME}
			PATHS ${Wasmtime_DOWNLOAD_LIBRARY_PATH}
			NO_DEFAULT_PATH
			DOC "Wasmtime C API library"
		)

		if(Wasmtime_LIBRARY)
			set(Wasmtime_LIBRARY_INSTALLED ON)

			if(PROJECT_OS_WIN)
				string(REGEX REPLACE "[.]lib$" ".dll" Wasmtime_LIBRARY_DLL ${Wasmtime_LIBRARY})
			elseif(PROJECT_OS_MINGW)
				string(REGEX REPLACE "[.]a$" ".dll" Wasmtime_LIBRARY_DLL ${Wasmtime_LIBRARY})
			endif()
		endif()
	endif()

	if(NOT Wasmtime_INCLUDE_DIR)
		find_path(Wasmtime_INCLUDE_DIR
			NAMES wasm.h wasmtime.h
			PATHS ${Wasmtime_DOWNLOAD_INCLUDE_DIR}
			NO_DEFAULT_PATH
			DOC "Wasmtime C API headers"
		)
	endif()
endif()

# Copy Wasmtime DLL to the output directory
if(Wasmtime_LIBRARY_DLL)
	file(COPY "${Wasmtime_LIBRARY_DLL}" DESTINATION "${PROJECT_OUTPUT_DIR}")
	set(Wasmtime_LIBRARY_DEPENDENCIES
		ws2_32.lib
		advapi32.lib
		userenv.lib
		ntdll.lib
		shell32.lib
		ole32.lib
		bcrypt.lib
	)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Wasmtime
	FOUND_VAR Wasmtime_FOUND
	REQUIRED_VARS Wasmtime_LIBRARY Wasmtime_INCLUDE_DIR
	VERSION_VAR Wasmtime_VERSION
)

mark_as_advanced(
	Wasmtime_FOUND
	Wasmtime_LIBRARY
	Wasmtime_INCLUDE_DIR
)

if(Wasmtime_CMAKE_DEBUG)
	message(STATUS "Found Wasmtime C API library at ${Wasmtime_LIBRARY}")
	message(STATUS "Found Wasmtime C API header in ${Wasmtime_INCLUDE_DIR}")
endif()
