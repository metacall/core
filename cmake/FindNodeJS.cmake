#
# CMake Find NodeJS JavaScript Runtime by Parra Studios
# Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Find NodeJS executable and include paths
#
# NodeJS_FOUND - True if NodeJS was found
# NodeJS_INCLUDE_DIRS - NodeJS headers paths
# NodeJS_VERSION - NodeJS version
# NodeJS_VERSION_MAJOR - NodeJS major version
# NodeJS_VERSION_MINOR - NodeJS minor version
# NodeJS_VERSION_PATCH - NodeJS patch version
# NodeJS_MODULE_VERSION - NodeJS module version
# NodeJS_UV_VERSION - UV version of NodeJS
# NodeJS_UV_VERSION_MAJOR - UV major version of NodeJS
# NodeJS_UV_VERSION_MINOR - UV minor version of NodeJS
# NodeJS_UV_VERSION_PATCH - UV patch version of NodeJS
# NodeJS_V8_VERSION - V8 version of NodeJS
# NodeJS_V8_VERSION_MAJOR - V8 major version of NodeJS
# NodeJS_V8_VERSION_MINOR - V8 minor version of NodeJS
# NodeJS_V8_VERSION_PATCH - V8 patch version of NodeJS
# NodeJS_V8_VERSION_TWEAK - V8 patch version of NodeJS
# NodeJS_V8_VERSION_HEX - V8 version of NodeJS in hexadecimal format
# NodeJS_LIBRARY - NodeJS shared library
# NodeJS_EXECUTABLE - NodeJS shell
#
# Configuration variables:
#
# NodeJS_CMAKE_DEBUG - Print paths for debugging
# NodeJS_EXECUTABLE_ONLY - Find only NodeJS executable (avoid library and include files)
# NodeJS_SHARED_UV - If it is enabled, libuv won't be required by this script
# NodeJS_BUILD_FROM_SOURCE - If it is enabled, NodeJS runtime library will be built from source
# NodeJS_INSTALL_PREFIX - Define a custom install prefix for NodeJS (Linux / Darwin only)

# Prevent vervosity if already included
if(NodeJS_EXECUTABLE)
	set(NodeJS_FIND_QUIETLY TRUE)
endif()

option(NodeJS_CMAKE_DEBUG "Print paths for debugging NodeJS dependencies." OFF)
option(NodeJS_SHARED_UV "If it is enabled, libuv won't be required by this script." OFF)
option(NodeJS_BUILD_FROM_SOURCE "If it is enabled, NodeJS runtime library will be built from source." OFF)

# Include package manager
include(FindPackageHandleStandardArgs)

# NodeJS paths
set(NodeJS_PATHS
	${NODE_HOME}
	${NODE_ROOT}
	$ENV{ProgramFiles}/node
	$ENV{SystemDrive}/node
	$ENV{NODE_HOME}
	$ENV{EXTERNLIBS}/node
	${NODE_DIR}
	$ENV{NODE_DIR}
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

# Find NodeJS include directories
if(MSVC OR CMAKE_BUILD_TYPE EQUAL "Debug")
	set(NodeJS_V8_HEADERS v8.h v8-version.h v8-profiler.h) # v8-debug.h
else()
	set(NodeJS_V8_HEADERS v8.h v8-version.h)
endif()

if(NOT NodeJS_SHARED_UV)
	set(NodeJS_UV_HEADERS uv.h)
endif()

set(NodeJS_HEADERS
	node.h
	node_api.h
)

set(NodeJS_INCLUDE_SUFFIXES
	include
	include/node
	include/nodejs
	include/src
	include/deps/v8/include
	include/nodejs/src
	include/nodejs/deps/v8/include
	include/nodejs/deps/uv/include
	src
)

set(NodeJS_INCLUDE_PATHS
	/usr
	/usr/local
)

# Find NodeJS executable
find_program(NodeJS_EXECUTABLE
	NAMES node nodejs node.exe
	HINTS ${NodeJS_PATHS}
	PATH_SUFFIXES bin
	DOC "NodeJS JavaScript Runtime Interpreter"
)

if(NodeJS_EXECUTABLE)
	# Detect NodeJS version
	execute_process(COMMAND ${NodeJS_EXECUTABLE} --version
		OUTPUT_VARIABLE NodeJS_VERSION_TAG
		RESULT_VARIABLE NodeJS_VERSION_TAG_RESULT
	)

	if(NodeJS_VERSION_TAG_RESULT EQUAL 0)
		string(REPLACE "v" "" NodeJS_VERSION_TAG "${NodeJS_VERSION_TAG}")
		string(REPLACE "\n" "" NodeJS_VERSION_TAG "${NodeJS_VERSION_TAG}")
		set(NodeJS_VERSION "${NodeJS_VERSION_TAG}")
		string(REPLACE "." ";" NodeJS_VERSION_LIST "${NodeJS_VERSION}")
		list(GET NodeJS_VERSION_LIST 0 NodeJS_VERSION_MAJOR)
		list(GET NodeJS_VERSION_LIST 1 NodeJS_VERSION_MINOR)
		list(GET NodeJS_VERSION_LIST 2 NodeJS_VERSION_PATCH)
	endif()

	# Detect UV version
	execute_process(COMMAND ${NodeJS_EXECUTABLE} -e "console.log(process.versions.uv)"
		OUTPUT_VARIABLE NodeJS_UV_VERSION_TAG
		RESULT_VARIABLE NodeJS_UV_VERSION_TAG_RESULT
	)

	if(NodeJS_VERSION_TAG_RESULT EQUAL 0)
		string(REPLACE "\n" "" NodeJS_UV_VERSION_TAG "${NodeJS_UV_VERSION_TAG}")
		set(NodeJS_UV_VERSION "${NodeJS_UV_VERSION_TAG}")
		string(REPLACE "." ";" NodeJS_UV_VERSION_LIST "${NodeJS_UV_VERSION}")
		list(GET NodeJS_UV_VERSION_LIST 0 NodeJS_UV_VERSION_MAJOR)
		list(GET NodeJS_UV_VERSION_LIST 1 NodeJS_UV_VERSION_MINOR)
		list(GET NodeJS_UV_VERSION_LIST 2 NodeJS_UV_VERSION_PATCH)
	endif()

	# TODO: Implement V8 version by command?

	# Check if NodeJS executable only is requested
	if(NodeJS_EXECUTABLE_ONLY)
		find_package_handle_standard_args(NodeJS
			REQUIRED_VARS NodeJS_EXECUTABLE
			VERSION_VAR NodeJS_VERSION
		)

		mark_as_advanced(NodeJS_EXECUTABLE)

		if(NodeJS_CMAKE_DEBUG)
			message(STATUS "NodeJS_VERSION: ${NodeJS_VERSION}")
			message(STATUS "NodeJS_EXECUTABLE: ${NodeJS_EXECUTABLE}")
		endif()

		return()
	endif()
endif()

if(NOT NodeJS_INCLUDE_DIR)
	# Find NodeJS includes
	find_path(NodeJS_INCLUDE_DIR
		NAMES ${NodeJS_HEADERS}
		PATHS ${NodeJS_INCLUDE_PATHS}
		PATH_SUFFIXES ${NodeJS_INCLUDE_SUFFIXES}
		DOC "NodeJS JavaScript Runtime Headers"
	)
endif()

# Check if the include directory contains all headers in the same folder
if(NodeJS_INCLUDE_DIR)
	foreach(HEADER IN ITEMS ${NodeJS_HEADERS})
		if(NOT EXISTS ${NodeJS_INCLUDE_DIR}/${HEADER})
			message(WARNING "NodeJS header ${HEADER} not found in ${NodeJS_INCLUDE_DIR}")
			unset(NodeJS_INCLUDE_DIR CACHE)
			break()
		endif()
	endforeach()
endif()

# Find NodeJS V8 includes
if(NOT NodeJS_V8_INCLUDE_DIR)
	find_path(NodeJS_V8_INCLUDE_DIR
		NAMES ${NodeJS_V8_HEADERS}
		PATHS ${NodeJS_INCLUDE_PATHS}
		PATH_SUFFIXES ${NodeJS_INCLUDE_SUFFIXES}
		DOC "NodeJS JavaScript Runtime V8 Headers"
	)
endif()

# Check if the include directory contains all headers in the same folder
if(NodeJS_V8_INCLUDE_DIR)
	foreach(HEADER IN ITEMS ${NodeJS_V8_HEADERS})
		if(NOT EXISTS ${NodeJS_V8_INCLUDE_DIR}/${HEADER})
			message(WARNING "NodeJS header ${HEADER} not found in ${NodeJS_V8_INCLUDE_DIR}")
			unset(NodeJS_V8_INCLUDE_DIR CACHE)
			break()
		endif()
	endforeach()
endif()

# Find NodeJS UV includes
if(NOT NodeJS_UV_INCLUDE_DIR)
	find_path(NodeJS_UV_INCLUDE_DIR
		NAMES ${NodeJS_UV_HEADERS}
		PATHS ${NodeJS_INCLUDE_PATHS}
		PATH_SUFFIXES ${NodeJS_INCLUDE_SUFFIXES}
		DOC "NodeJS JavaScript Runtime UV Headers"
	)
endif()

# Check if the include directory contains all headers in the same folder
if(NodeJS_UV_INCLUDE_DIR)
	foreach(HEADER IN ITEMS ${NodeJS_UV_HEADERS})
		if(NOT EXISTS ${NodeJS_UV_INCLUDE_DIR}/${HEADER})
			message(WARNING "NodeJS header ${HEADER} not found in ${NodeJS_UV_INCLUDE_DIR}")
			unset(NodeJS_UV_INCLUDE_DIR CACHE)
			break()
		endif()
	endforeach()
endif()

# Download includes in case they are not distributed
if(NOT NodeJS_INCLUDE_DIR OR NOT NodeJS_V8_INCLUDE_DIR OR NOT NodeJS_UV_INCLUDE_DIR)
	if(NOT NodeJS_VERSION)
		# We do not have any way to know what version to install
		message(WARNING "NodeJS headers could not be found, neither a valid NodeJS version.")
		return()
	endif()

	# NodeJS download and output path (workaround for NodeJS headers)
	set(NodeJS_DOWNLOAD_URL "https://nodejs.org/dist/v${NodeJS_VERSION}/node-v${NodeJS_VERSION}-headers.tar.gz")
	set(NodeJS_BASE_PATH "${CMAKE_CURRENT_BINARY_DIR}/headers")
	set(NodeJS_DOWNLOAD_FILE "${NodeJS_BASE_PATH}/node-v${NodeJS_VERSION}-headers.tar.gz")
	set(NodeJS_HEADERS_OUTPUT_PATH "${NodeJS_BASE_PATH}/node-v${NodeJS_VERSION}")

	# Download node if needed
	if(NOT EXISTS "${NodeJS_DOWNLOAD_FILE}")
		message(STATUS "Downloading NodeJS headers")
		file(DOWNLOAD ${NodeJS_DOWNLOAD_URL} ${NodeJS_DOWNLOAD_FILE} SHOW_PROGRESS)
	endif()

	# Decompress node if needed
	if(NOT EXISTS "${NodeJS_HEADERS_OUTPUT_PATH}")
		message(STATUS "Extract NodeJS headers")
		execute_process(COMMAND ${CMAKE_COMMAND} -E tar "xvf" "${NodeJS_DOWNLOAD_FILE}" WORKING_DIRECTORY "${NodeJS_BASE_PATH}" OUTPUT_QUIET)
	endif()

	# Find NodeJS includes
	find_path(NodeJS_INCLUDE_DIR
		NAMES ${NodeJS_HEADERS}
		PATHS ${NodeJS_HEADERS_OUTPUT_PATH}
		PATH_SUFFIXES ${NodeJS_INCLUDE_SUFFIXES}
		DOC "NodeJS JavaScript Runtime Headers"
		NO_CMAKE_SYSTEM_PATH
		NO_SYSTEM_ENVIRONMENT_PATH
	)

	# Find NodeJS V8 includes
	find_path(NodeJS_V8_INCLUDE_DIR
		NAMES ${NodeJS_V8_HEADERS}
		PATHS ${NodeJS_HEADERS_OUTPUT_PATH}
		PATH_SUFFIXES ${NodeJS_INCLUDE_SUFFIXES}
		DOC "NodeJS JavaScript Runtime V8 Headers"
		NO_CMAKE_SYSTEM_PATH
		NO_SYSTEM_ENVIRONMENT_PATH
	)

	# Find NodeJS UV includes
	find_path(NodeJS_UV_INCLUDE_DIR
		NAMES ${NodeJS_UV_HEADERS}
		PATHS ${NodeJS_HEADERS_OUTPUT_PATH}
		PATH_SUFFIXES ${NodeJS_INCLUDE_SUFFIXES}
		DOC "NodeJS JavaScript Runtime UV Headers"
		NO_CMAKE_SYSTEM_PATH
		NO_SYSTEM_ENVIRONMENT_PATH
	)
endif()

if(NodeJS_INCLUDE_DIR)
	# Get node module version
	find_file(NodeJS_VERSION_FILE_PATH node_version.h
		PATHS ${NodeJS_INCLUDE_DIR}
		PATH_SUFFIXES ${NodeJS_INCLUDE_SUFFIXES}
		DOC "NodeJS JavaScript Version Header"
	)

	if(NodeJS_VERSION_FILE_PATH)
		file(READ ${NodeJS_VERSION_FILE_PATH} NodeJS_VERSION_FILE)

		string(REGEX MATCH "#define NODE_MODULE_VERSION ([0-9]+)" NodeJS_MODULE_VERSION_DEF ${NodeJS_VERSION_FILE})
		string(REGEX MATCH "([0-9]+)$" NodeJS_MODULE_VERSION ${NodeJS_MODULE_VERSION_DEF})
	endif()
endif()

if(NodeJS_V8_INCLUDE_DIR)
	# Detect NodeJS V8 version
	find_file(NodeJS_V8_VERSION_FILE_PATH v8-version.h
		PATHS ${NodeJS_V8_INCLUDE_DIR}
		PATH_SUFFIXES ${NodeJS_INCLUDE_SUFFIXES}
		DOC "NodeJS V8 JavaScript Version Header"
	)

	if(NodeJS_V8_VERSION_FILE_PATH)
		file(READ ${NodeJS_V8_VERSION_FILE_PATH} NodeJS_V8_VERSION_FILE)

		string(REGEX MATCH "#define V8_MAJOR_VERSION ([0-9]+)" NodeJS_V8_VERSION_MAJOR_DEF ${NodeJS_V8_VERSION_FILE})
		string(REGEX MATCH "([0-9]+)$" NodeJS_V8_VERSION_MAJOR ${NodeJS_V8_VERSION_MAJOR_DEF})

		string(REGEX MATCH "#define V8_MINOR_VERSION ([0-9]+)" NodeJS_V8_VERSION_MINOR_DEF ${NodeJS_V8_VERSION_FILE})
		string(REGEX MATCH "([0-9]+)$" NodeJS_V8_VERSION_MINOR ${NodeJS_V8_VERSION_MINOR_DEF})

		string(REGEX MATCH "#define V8_BUILD_NUMBER ([0-9]+)" NodeJS_V8_VERSION_PATCH_DEF ${NodeJS_V8_VERSION_FILE})
		string(REGEX MATCH "([0-9]+)$" NodeJS_V8_VERSION_PATCH ${NodeJS_V8_VERSION_PATCH_DEF})

		string(REGEX MATCH "#define V8_PATCH_LEVEL ([0-9]+)" NodeJS_V8_VERSION_TWEAK_DEF ${NodeJS_V8_VERSION_FILE})
		string(REGEX MATCH "([0-9]+)$" NodeJS_V8_VERSION_TWEAK ${NodeJS_V8_VERSION_TWEAK_DEF})

		set(NodeJS_V8_VERSION "${NodeJS_V8_VERSION_MAJOR}.${NodeJS_V8_VERSION_MINOR}.${NodeJS_V8_VERSION_PATCH}.${NodeJS_V8_VERSION_TWEAK}")

		set(NodeJS_V8_VERSION_HEX 0x0${NodeJS_V8_VERSION_MAJOR}${NodeJS_V8_VERSION_MINOR}${NodeJS_V8_VERSION_PATCH}${NodeJS_V8_VERSION_TWEAK})
		string(LENGTH "${NodeJS_V8_VERSION_HEX}" NodeJS_V8_VERSION_HEX_LENGTH)

		while(NodeJS_V8_VERSION_HEX_LENGTH LESS 8)

			set(NodeJS_V8_VERSION_HEX "${NodeJS_V8_VERSION_HEX}0")
			string(LENGTH "${NodeJS_V8_VERSION_HEX}" NodeJS_V8_VERSION_HEX_LENGTH)

		endwhile()
	endif()
endif()

# Find NodeJS library from module version
if(NodeJS_MODULE_VERSION)
	# NodeJS library names
	if(WIN32)
		if(NodeJS_VERSION_MAJOR GREATER_EQUAL 14)
			set(NodeJS_LIBRARY_NAMES
				libnode.${NodeJS_MODULE_VERSION}.dll
				libnode.dll
				libnode.lib
			)
		else()
			set(NodeJS_LIBRARY_NAMES
				node.${NodeJS_MODULE_VERSION}.dll
				node.dll
				node.lib
			)
		endif()
	else()
		set(NodeJS_LIBRARY_NAMES
			libnode.so.${NodeJS_MODULE_VERSION}
			libnode.so
			libnode.${NodeJS_MODULE_VERSION}.dylib
			libnode.dylib
		)
	endif()

	if(NOT NodeJS_BUILD_FROM_SOURCE)
		message(STATUS "Searching NodeJS library version ${NodeJS_MODULE_VERSION}")

		if(WIN32)
			set(NodeJS_LIBRARY_PATH "C:/Program Files/nodejs")
		else()
			set(NodeJS_LIBRARY_PATH "/usr/local/lib")
		endif()

		set(NodeJS_SYSTEM_LIBRARY_PATH "/lib/x86_64-linux-gnu" "/usr/lib/x86_64-linux-gnu") # TODO: Add others

		# Find library
		find_library(NodeJS_LIBRARY
			NAMES ${NodeJS_LIBRARY_NAMES}
			PATHS ${NodeJS_LIBRARY_PATH} ${NodeJS_SYSTEM_LIBRARY_PATH}
			DOC "NodeJS JavaScript Runtime Library"
		)

		if(NodeJS_LIBRARY)
			message(STATUS "NodeJS Library Found")
		endif()
	endif()
endif()

# Install NodeJS library in case it is not distributed
if(NOT NodeJS_LIBRARY)
	message(STATUS "NodeJS library not found, trying to build it from source")

	# NodeJS download and output path (workaround to compile node as a shared library)
	set(NodeJS_DOWNLOAD_URL "https://nodejs.org/dist/v${NodeJS_VERSION}/node-v${NodeJS_VERSION}.tar.gz")
	set(NodeJS_BASE_PATH "${CMAKE_CURRENT_BINARY_DIR}/sources")
	set(NodeJS_DOWNLOAD_FILE "${NodeJS_BASE_PATH}/node-v${NodeJS_VERSION}.tar.gz")
	set(NodeJS_OUTPUT_PATH "${NodeJS_BASE_PATH}/node-v${NodeJS_VERSION}")

	# Download node if needed
	if(NOT EXISTS "${NodeJS_DOWNLOAD_FILE}")
		message(STATUS "Downloading NodeJS distribution v${NodeJS_VERSION}")
		file(DOWNLOAD ${NodeJS_DOWNLOAD_URL} ${NodeJS_DOWNLOAD_FILE} SHOW_PROGRESS)
	endif()

	# Decompress node if needed
	if(NOT EXISTS "${NodeJS_OUTPUT_PATH}")
		message(STATUS "Extract NodeJS distribution")
		execute_process(COMMAND ${CMAKE_COMMAND} -E tar "xvf" "${NodeJS_DOWNLOAD_FILE}" WORKING_DIRECTORY "${NodeJS_BASE_PATH}" OUTPUT_QUIET)
	endif()

	if(WIN32)
		if(NodeJS_VERSION_MAJOR LESS 14)
			set(NodeJS_COMPILE_PATH "${NodeJS_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}")
		else()
			set(NodeJS_COMPILE_PATH "${NodeJS_OUTPUT_PATH}/out/${CMAKE_BUILD_TYPE}")
		endif()
	else()
		if(NodeJS_VERSION_MAJOR LESS 14)
			set(NodeJS_COMPILE_PATH "${NodeJS_OUTPUT_PATH}/out")
		else()
			set(NodeJS_COMPILE_PATH "${NodeJS_OUTPUT_PATH}/out/${CMAKE_BUILD_TYPE}")
		endif()
	endif()

	# Compile node as a shared library if needed
	if(NOT EXISTS "${NodeJS_COMPILE_PATH}")
		if(WIN32 AND MSVC)
			if(NOT EXISTS "${NodeJS_COMPILE_PATH}/node.dll" AND NOT EXISTS "${NodeJS_COMPILE_PATH}/libnode.dll")
				message(STATUS "Build NodeJS shared library")

				if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
					set(NodeJS_COMPILE_ARCH "x86")
				else()
					set(NodeJS_COMPILE_ARCH "x64")
				endif()

				# Check for Visual Studio Version and configure the build command
				if(MSVC_VERSION GREATER 1916)
					set(NodeJS_MSVC_VER vs2019)
				elseif(MSVC_VERSION GREATER 1900)
					set(NodeJS_MSVC_VER vs2017)
				elseif(MSVC_VERSION EQUAL 1900)
					set(NodeJS_MSVC_VER vs2015)
				else()
					message(FATAL_ERROR "Version of Visual Studio too old, add other toolsets in FindNodeJS.cmake in order to support them")
				endif()

				if(CMAKE_BUILD_TYPE STREQUAL "Debug")
					set(NodeJS_BUILD_TYPE debug)
				else()
					set(NodeJS_BUILD_TYPE release)
				endif()

				# Building NodeJS 14 as library in Windows is broken (so we need to patch it)
				if(WIN32 AND NodeJS_VERSION_MAJOR GREATER_EQUAL 14)
					find_package(Python COMPONENTS Interpreter REQUIRED)

					execute_process(
						COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cmake/NodeJSGYPPatch.py ${NodeJS_OUTPUT_PATH}/node.gyp
						WORKING_DIRECTORY "${NodeJS_OUTPUT_PATH}"
						RESULT_VARIABLE NodeJS_PATCH_SCRIPT
					)

					if(NOT NodeJS_PATCH_SCRIPT EQUAL 0)
						message(FATAL_ERROR "FindNodeJS.cmake failed to patch node.gyp project")
					endif()
				endif()

				execute_process(
					COMMAND vcbuild.bat dll ${NodeJS_BUILD_TYPE} ${NodeJS_COMPILE_ARCH} ${NodeJS_MSVC_VER}
					WORKING_DIRECTORY "${NodeJS_OUTPUT_PATH}"
				)

				if(EXISTS "${NodeJS_COMPILE_PATH}/node.dll")
					set(NodeJS_LIBRARY_NAME "node.dll")
				elseif(EXISTS "${NodeJS_COMPILE_PATH}/libnode.dll")
					set(NodeJS_LIBRARY_NAME "libnode.dll")
				else()
					message(FATAL_ERROR "FindNodeJS.cmake failed to build node library")
				endif()

				# Copy library to MetaCall output path
				execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_OUTPUT_DIR})
				file(COPY ${NodeJS_COMPILE_PATH}/${NodeJS_LIBRARY_NAME} DESTINATION ${PROJECT_OUTPUT_DIR})

				message(STATUS "Install NodeJS shared library")
			endif()
		else()
			message(STATUS "Configure NodeJS shared library")

			# Select the ICU library depending on the NodeJS version
			if("${NodeJS_VERSION_MAJOR}" GREATER_EQUAL "18")
				set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-71-1/icu4c-71_1-src.zip")
			elseif("${NodeJS_VERSION_MAJOR}" GREATER_EQUAL "16")
				set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-69-1/icu4c-69_1-src.zip")
			elseif("${NodeJS_VERSION_MAJOR}" GREATER_EQUAL "15")
				set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-67-1/icu4c-67_1-src.zip")
			elseif("${NodeJS_VERSION_MAJOR}" GREATER_EQUAL "14")
				set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-66-1/icu4c-66_1-src.zip")
			elseif("${NodeJS_VERSION_MAJOR}" GREATER_EQUAL "12")
				set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-65-1/icu4c-65_1-src.zip")
			elseif("${NodeJS_VERSION_MAJOR}" GREATER_EQUAL "10")
				set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-64-2/icu4c-64_2-src.zip")
			endif()

			if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
				set(ICU_DEBUG --debug)
			else()
				set(ICU_DEBUG)
			endif()

			# Workaround for OpenSSL bug: https://github.com/metacall/core/issues/223
			if(APPLE)
				set(ICU_ENV_VAR ${CMAKE_COMMAND} -E env PYTHONHTTPSVERIFY=0)
			else()
				set(ICU_ENV_VAR)
			endif()

			if(NodeJS_INSTALL_PREFIX)
				set(NodeJS_PREFIX "--prefix=${NodeJS_INSTALL_PREFIX}")
			else()
				set(NodeJS_PREFIX)
			endif()

			execute_process(COMMAND ${ICU_ENV_VAR} sh -c "./configure ${NodeJS_PREFIX} --with-icu-source=${ICU_URL} --shared ${ICU_DEBUG}" WORKING_DIRECTORY "${NodeJS_OUTPUT_PATH}")

			message(STATUS "Build NodeJS shared library")

			include(ProcessorCount)

			ProcessorCount(N)

			if(NOT N EQUAL 0)
				execute_process(COMMAND sh -c "make -j${N} -C out BUILDTYPE=${CMAKE_BUILD_TYPE} V=1" WORKING_DIRECTORY "${NodeJS_OUTPUT_PATH}")
			else()
				execute_process(COMMAND sh -c "make -C out BUILDTYPE=${CMAKE_BUILD_TYPE} V=1" WORKING_DIRECTORY "${NodeJS_OUTPUT_PATH}")
			endif()

			message(STATUS "Install NodeJS shared library")

			execute_process(COMMAND sh -c "make install" WORKING_DIRECTORY "${NodeJS_OUTPUT_PATH}")
		endif()
	endif()

	# Set up the compile path in case of prefix is specified
	if(NOT WIN32 AND NOT MSVC AND NodeJS_INSTALL_PREFIX)
		set(NodeJS_COMPILE_PATH "${NodeJS_INSTALL_PREFIX}/lib")
	endif()

	# Find compiled library
	find_library(NodeJS_LIBRARY
		NAMES ${NodeJS_LIBRARY_NAMES}
		PATHS ${NodeJS_COMPILE_PATH}
		DOC "NodeJS JavaScript Runtime Library"
		NO_CMAKE_SYSTEM_PATH
		NO_SYSTEM_ENVIRONMENT_PATH
	)

	if(NOT NodeJS_LIBRARY)
		message(FATAL_ERROR "NodeJS library not found and it could not be built from source")
	endif()
endif()

set(NodeJS_INCLUDE_DIRS "${NodeJS_INCLUDE_DIR}")

if(NodeJS_V8_INCLUDE_DIR)
	set(NodeJS_INCLUDE_DIRS "${NodeJS_INCLUDE_DIRS}" "${NodeJS_V8_INCLUDE_DIR}")
endif()

if(NodeJS_UV_INCLUDE_DIR)
	set(NodeJS_INCLUDE_DIRS "${NodeJS_INCLUDE_DIRS}" "${NodeJS_UV_INCLUDE_DIR}")
endif()

if(NOT NodeJS_LIBRARY_NAME)
	get_filename_component(NodeJS_LIBRARY_NAME "${NodeJS_LIBRARY}" NAME)
endif()

find_package_handle_standard_args(NodeJS
	REQUIRED_VARS NodeJS_EXECUTABLE NodeJS_INCLUDE_DIRS NodeJS_LIBRARY NodeJS_LIBRARY_NAME
	VERSION_VAR NodeJS_VERSION
)

mark_as_advanced(NodeJS_EXECUTABLE NodeJS_INCLUDE_DIRS NodeJS_LIBRARY NodeJS_LIBRARY_NAME)

if(NodeJS_CMAKE_DEBUG)
	message(STATUS "NodeJS_INCLUDE_DIRS: ${NodeJS_INCLUDE_DIRS}")
	message(STATUS "NodeJS_VERSION: ${NodeJS_VERSION}")
	message(STATUS "NodeJS_UV_VERSION: ${NodeJS_UV_VERSION}")
	message(STATUS "NodeJS_V8_VERSION: ${NodeJS_V8_VERSION}")
	message(STATUS "NodeJS_V8_VERSION_HEX: ${NodeJS_V8_VERSION_HEX}")
	message(STATUS "NodeJS_LIBRARY: ${NodeJS_LIBRARY}")
	message(STATUS "NodeJS_EXECUTABLE: ${NodeJS_EXECUTABLE}")
endif()
