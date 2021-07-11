#
# CMake Find Wasmtime WebAssembly Runtime by Parra Studios
# Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Find Wasmtime library and include paths
#
# WASMTIME_LIBRARY - Wasmtime shared library
# WASMTIME_INCLUDE_DIR - Wasmtime include directory

include(Portability)

set(WASMTIME_VERSION ${Wasmtime_FIND_VERSION})

# See https://docs.wasmtime.dev/contributing-building.html#building-the-wasmtime-c-api
if(PROJECT_OS_LINUX)
	set(WASMTIME_LIBRARY_NAME libwasmtime.so)
	set(WASMTIME_PLATFORM "linux")
	set(WASMTIME_ARCHIVE_EXTENSION "tar.xz")
elseif(PROJECT_OS_FAMILY STREQUAL "macos")
	set(WASMTIME_LIBRARY_NAME libwasmtime.dylib)
	set(WASMTIME_PLATFORM "macos")
	set(WASMTIME_ARCHIVE_EXTENSION "tar.xz")
elseif(PROJECT_OS_WIN)
	set(WASMTIME_LIBRARY_NAME wasmtime.dll)
	set(WASMTIME_PLATFORM "windows")
	set(WASMTIME_ARCHIVE_EXTENSION "zip")
elseif(PROJECT_OS_MINGW)
	set(WASMTIME_LIBRARY_NAME wasmtime.dll)
	set(WASMTIME_PLATFORM "mingw")
	set(WASMTIME_ARCHIVE_EXTENSION "zip")
else()
	message(FATAL_ERROR "Could not determine target platform or target platform is not supported")
endif()

if(PROJECT_ARCH_AMD64)
	set(WASMTIME_ARCH "x86_64")
elseif(PROJECT_ARCH_AARCH64)
	set(WASMTIME_ARCH "aarch64")
else()
	set(WASMTIME_ARCH "x86_64")
	message(WARNING "Could not determine target architecture, assuming x86_64")
endif()

message(DEBUG "Set target architecture to ${WASMTIME_ARCH}")

set(WASMTIME_LOCAL_PATH "${CMAKE_BINARY_DIR}/wasmtime")
set(WASMTIME_DOWNLOAD_DIR_NAME "wasmtime-v${WASMTIME_VERSION}-${WASMTIME_ARCH}-${WASMTIME_PLATFORM}-c-api")
set(WASMTIME_API_PATH "${WASMTIME_LOCAL_PATH}/${WASMTIME_DOWNLOAD_DIR_NAME}")
set(WASMTIME_DOWNLOAD_LIBRARY_PATH "${WASMTIME_API_PATH}/lib")
set(WASMTIME_DOWNLOAD_INCLUDE_DIR "${WASMTIME_API_PATH}/include")

find_library(WASMTIME_LIBRARY
	NAMES ${WASMTIME_LIBRARY_NAME}
	PATHS ${WASMTIME_DOWNLOAD_LIBRARY_PATH}
	DOC "Wasmtime C API library"
)

find_path(WASMTIME_INCLUDE_DIR
	NAMES wasm.h
	PATHS ${WASMTIME_DOWNLOAD_INCLUDE_DIR}
	DOC "Wasm C API header"
)

if(WASMTIME_LIBRARY AND WASMTIME_INCLUDE_DIR)
	message(DEBUG "Found Wasmtime C API library at ${WASMTIME_LIBRARY}")
	message(DEBUG "Found Wasm C API header in ${WASMTIME_INCLUDE_DIR}")
else()
	message(STATUS "Wasmtime C API library or header not found, downloading from archive")

	if(WASMTIME_ARCH STREQUAL "x86_64" AND PROJECT_ARCH_32BIT)
		# We assumed target architecture was x86_64, but it is 32-bit, so the
		# assumption is definitely invalid.
		message(FATAL_ERROR "No downloads available for target architecture, please install Wasmtime manually")
	endif()

	set(WASMTIME_DOWNLOAD_ARCHIVE_NAME "${WASMTIME_DOWNLOAD_DIR_NAME}.${WASMTIME_ARCHIVE_EXTENSION}")
	set(WASMTIME_DOWNLOAD_URL
		"https://github.com/bytecodealliance/wasmtime/releases/download/v${WASMTIME_VERSION}/${WASMTIME_DOWNLOAD_ARCHIVE_NAME}")
	set(WASMTIME_DOWNLOAD_DEST "${WASMTIME_LOCAL_PATH}/${WASMTIME_DOWNLOAD_ARCHIVE_NAME}")

	file(DOWNLOAD ${WASMTIME_DOWNLOAD_URL} ${WASMTIME_DOWNLOAD_DEST}
		STATUS DOWNLOAD_STATUS
		SHOW_PROGRESS)

	list(GET DOWNLOAD_STATUS 0 STATUS_CODE)

	if(NOT STATUS_CODE EQUAL 0)
		list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
		message(FATAL_ERROR "Failed to download Wasmtime C API: ${ERROR_MESSAGE}")
	endif()

	file(ARCHIVE_EXTRACT
		INPUT ${WASMTIME_DOWNLOAD_DEST}
		DESTINATION ${WASMTIME_LOCAL_PATH}
	)

	find_library(WASMTIME_LIBRARY
		NAMES ${WASMTIME_LIBRARY_NAME}
		PATHS ${WASMTIME_DOWNLOAD_LIBRARY_PATH}
		NO_DEFAULT_PATH
		DOC "Wasmtime C API library"
	)

	find_path(WASMTIME_INCLUDE_DIR
		NAMES wasm.h
		PATHS ${WASMTIME_DOWNLOAD_INCLUDE_DIR}
		NO_DEFAULT_PATH
		DOC "Wasm C API header"
	)
endif()
