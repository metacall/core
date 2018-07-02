#
# CMake Find NodeJS JavaScript Runtime by Parra Studios
# Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Find NodeJS executable and include paths
#
# NODEJS_FOUND - True if NodeJS was found
# NODEJS_INCLUDE_DIR - NodeJS headers path
# NODEJS_VERSION - NodeJS version
# NODEJS_VERSION_MAJOR - NodeJS major version
# NODEJS_VERSION_MINOR - NodeJS minor version
# NODEJS_VERSION_PATCH - NodeJS patch version
# NODEJS_MODULE_VERSION - NodeJS module version
# NODEJS_UV_VERSION - UV version of NodeJS
# NODEJS_UV_VERSION_MAJOR - UV major version of NodeJS
# NODEJS_UV_VERSION_MINOR - UV minor version of NodeJS
# NODEJS_UV_VERSION_PATCH - UV patch version of NodeJS
# NODEJS_V8_VERSION - V8 version of NodeJS
# NODEJS_V8_VERSION_MAJOR - V8 major version of NodeJS
# NODEJS_V8_VERSION_MINOR - V8 minor version of NodeJS
# NODEJS_V8_VERSION_PATCH - V8 patch version of NodeJS
# NODEJS_V8_VERSION_TWEAK - V8 patch version of NodeJS
# NODEJS_V8_VERSION_HEX - V8 version of NodeJS in hexadecimal format
# NODEJS_LIBRARY - NodeJS shared library
# NODEJS_EXECUTABLE - NodeJS shell

# Prevent vervosity if already included
if(NODEJS_INCLUDE_DIR)
	set(NODEJS_FIND_QUIETLY TRUE)
endif()

# Debug flag
set(_NODEJS_CMAKE_DEBUG TRUE)

# Include package manager
include(FindPackageHandleStandardArgs)

# NodeJS paths
set(NODEJS_PATHS
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
	$ENV{HOME}/.nvm/versions/node/v8.11.1 # TODO: Make this valid for all versions
)

# Find NodeJS include directories
if(MSVC OR CMAKE_BUILD_TYPE EQUAL "Debug")
	set(NODEJS_V8_HEADERS v8.h v8-debug.h v8-profiler.h v8stdint.h v8-version.h)
else()
	set(NODEJS_V8_HEADERS v8.h v8stdint.h v8-version.h)
endif()

set(NODEJS_UV_HEADERS uv.h) # TODO: Add uv-(platform).h?

find_path(NODEJS_UV_INCLUDE_DIR ${NODEJS_UV_HEADERS}
	PATHS /usr/include/compat-libuv010
	NO_DEFAULT_PATH
)

set(NODEJS_HEADERS
	node.h
	${NODEJS_V8_HEADERS}
)

set(NODEJS_INCLUDE_SUFFIXES
	include
	include/node
	include/nodejs
	include/src
	include/deps/v8/include
	include/nodejs/src
	include/nodejs/deps/v8/include
	include/nodejs/deps/uv/include
)

if(NOT NODEJS_UV_INCLUDE_DIR)
	set(NODEJS_HEADERS
		${NODEJS_HEADERS}
		${NODEJS_UV_HEADERS}
	)

	set(NODEJS_INCLUDE_SUFFIXES
		${NODEJS_INCLUDE_SUFFIXES}
		include/deps/uv/include
	)
endif()

find_path(NODEJS_INCLUDE_DIR ${NODEJS_HEADERS}
	PATHS ${NODEJS_INCLUDE_PATHS}
	PATH_SUFFIXES ${NODEJS_INCLUDE_SUFFIXES}
	DOC "NodeJS JavaScript Runtime Headers"
)

# Find NodeJS executable
find_program(NODEJS_EXECUTABLE
	NAMES node nodejs node.exe
	HINTS ${NODEJS_PATHS}
	PATH_SUFFIXES bin
	DOC "NodeJS JavaScript Runtime Interpreter"
)

if(NODEJS_EXECUTABLE)
	# Detect NodeJS version
	execute_process(COMMAND ${NODEJS_EXECUTABLE} --version
		OUTPUT_VARIABLE NODEJS_VERSION_TAG
		RESULT_VARIABLE NODEJS_VERSION_TAG_RESULT
	)

	if(NODEJS_VERSION_TAG_RESULT EQUAL 0)
		string(REPLACE "v" "" NODEJS_VERSION_TAG "${NODEJS_VERSION_TAG}")
		string(REPLACE "\n" "" NODEJS_VERSION_TAG "${NODEJS_VERSION_TAG}")
		set(NODEJS_VERSION "${NODEJS_VERSION_TAG}")
		string(REPLACE "." ";" NODEJS_VERSION_LIST "${NODEJS_VERSION}")
		list(GET NODEJS_VERSION_LIST 0 NODEJS_VERSION_MAJOR)
		list(GET NODEJS_VERSION_LIST 1 NODEJS_VERSION_MINOR)
		list(GET NODEJS_VERSION_LIST 2 NODEJS_VERSION_PATCH)
	endif()

	# Detect UV version
	execute_process(COMMAND ${NODEJS_EXECUTABLE} -e "console.log(process.versions.uv)"
		OUTPUT_VARIABLE NODEJS_UV_VERSION_TAG
		RESULT_VARIABLE NODEJS_UV_VERSION_TAG_RESULT
	)

	if(NODEJS_VERSION_TAG_RESULT EQUAL 0)
		string(REPLACE "\n" "" NODEJS_UV_VERSION_TAG "${NODEJS_UV_VERSION_TAG}")
		set(NODEJS_UV_VERSION "${NODEJS_UV_VERSION_TAG}")
		string(REPLACE "." ";" NODEJS_UV_VERSION_LIST "${NODEJS_UV_VERSION}")
		list(GET NODEJS_UV_VERSION_LIST 0 NODEJS_UV_VERSION_MAJOR)
		list(GET NODEJS_UV_VERSION_LIST 1 NODEJS_UV_VERSION_MINOR)
		list(GET NODEJS_UV_VERSION_LIST 2 NODEJS_UV_VERSION_PATCH)
	endif()

	# TODO: Implement V8 version by command?

endif()

# Detect NodeJS V8 version
if(NODEJS_INCLUDE_DIR)
	find_file(NODEJS_V8_VERSION_FILE_PATH v8-version.h
		PATHS ${NODEJS_INCLUDE_DIR}
		PATH_SUFFIXES ${NODEJS_INCLUDE_SUFFIXES}
		DOC "NodeJS V8 JavaScript Version Header"
	)

	if(NODEJS_V8_VERSION_FILE_PATH)
		file(READ ${NODEJS_V8_VERSION_FILE_PATH} NODEJS_V8_VERSION_FILE)

		string(REGEX MATCH "#define V8_MAJOR_VERSION ([0-9]+)" NODEJS_V8_VERSION_MAJOR_DEF ${NODEJS_V8_VERSION_FILE})
		string(REGEX MATCH "([0-9]+)$" NODEJS_V8_VERSION_MAJOR ${NODEJS_V8_VERSION_MAJOR_DEF})

		string(REGEX MATCH "#define V8_MINOR_VERSION ([0-9]+)" NODEJS_V8_VERSION_MINOR_DEF ${NODEJS_V8_VERSION_FILE})
		string(REGEX MATCH "([0-9]+)$" NODEJS_V8_VERSION_MINOR ${NODEJS_V8_VERSION_MINOR_DEF})

		string(REGEX MATCH "#define V8_BUILD_NUMBER ([0-9]+)" NODEJS_V8_VERSION_PATCH_DEF ${NODEJS_V8_VERSION_FILE})
		string(REGEX MATCH "([0-9]+)$" NODEJS_V8_VERSION_PATCH ${NODEJS_V8_VERSION_PATCH_DEF})

		string(REGEX MATCH "#define V8_PATCH_LEVEL ([0-9]+)" NODEJS_V8_VERSION_TWEAK_DEF ${NODEJS_V8_VERSION_FILE})
		string(REGEX MATCH "([0-9]+)$" NODEJS_V8_VERSION_TWEAK ${NODEJS_V8_VERSION_TWEAK_DEF})

		set(NODEJS_V8_VERSION "${NODEJS_V8_VERSION_MAJOR}.${NODEJS_V8_VERSION_MINOR}.${NODEJS_V8_VERSION_PATCH}.${NODEJS_V8_VERSION_TWEAK}")

		set(NODEJS_V8_VERSION_HEX 0x0${NODEJS_V8_VERSION_MAJOR}${NODEJS_V8_VERSION_MINOR}${NODEJS_V8_VERSION_PATCH}${NODEJS_V8_VERSION_TWEAK})
		string(LENGTH "${NODEJS_V8_VERSION_HEX}" NODEJS_V8_VERSION_HEX_LENGTH)

		while(NODEJS_V8_VERSION_HEX_LENGTH LESS 8)

			set(NODEJS_V8_VERSION_HEX "${NODEJS_V8_VERSION_HEX}0")
			string(LENGTH "${NODEJS_V8_VERSION_HEX}" NODEJS_V8_VERSION_HEX_LENGTH)

		endwhile()
	endif()

	find_file(NODEJS_VERSION_FILE_PATH node_version.h
		PATHS ${NODEJS_INCLUDE_DIR}
		PATH_SUFFIXES ${NODEJS_INCLUDE_SUFFIXES}
		DOC "NodeJS JavaScript Version Header"
	)

	if(NODEJS_VERSION_FILE_PATH)
		file(READ ${NODEJS_VERSION_FILE_PATH} NODEJS_VERSION_FILE)

		string(REGEX MATCH "#define NODE_MODULE_VERSION ([0-9]+)" NODEJS_MODULE_VERSION_DEF ${NODEJS_VERSION_FILE})
		string(REGEX MATCH "([0-9]+)$" NODEJS_MODULE_VERSION ${NODEJS_MODULE_VERSION_DEF})
	endif()
endif()

# TODO: Remove this workaround when NodeJS begins to distribute node as a shared library

# NodeJS library names
set(NODEJS_LIBRARY_NAMES
	libnode.so.${NODEJS_MODULE_VERSION}
	libnode.${NODEJS_MODULE_VERSION}.dylib
	libnode.${NODEJS_MODULE_VERSION}.dll
	node.lib
)

# NodeJS download and output path (workaround to compile node as a shared library)
set(NODEJS_DOWNLOAD_URL "https://nodejs.org/dist/v${NODEJS_VERSION}/node-v${NODEJS_VERSION}.tar.gz")
set(NODEJS_DOWNLOAD_FILE "${CMAKE_BINARY_DIR}/node-v${NODEJS_VERSION}.tar.gz")
set(NODEJS_OUTPUT_PATH "${CMAKE_BINARY_DIR}/node-v${NODEJS_VERSION}")

if(WIN32)
	set(NODEJS_COMPILE_PATH "${NODEJS_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}")
	set(NODEJS_LIBRARY_PATH "${NODEJS_COMPILE_PATH}")
else()
	set(NODEJS_COMPILE_PATH "${NODEJS_OUTPUT_PATH}/out/${CMAKE_BUILD_TYPE}")
	set(NODEJS_LIBRARY_PATH "${NODEJS_COMPILE_PATH}/lib.target")
endif()

# Download node if needed
if(NOT EXISTS "${NODEJS_DOWNLOAD_FILE}")
	message(STATUS "Downloading NodeJS distribution")
	file(DOWNLOAD ${NODEJS_DOWNLOAD_URL} ${NODEJS_DOWNLOAD_FILE})
endif()

# Decompress node if needed
if(NOT EXISTS "${NODEJS_OUTPUT_PATH}")
	message(STATUS "Extract NodeJS distribution")
	execute_process(COMMAND ${CMAKE_COMMAND} -E tar "xvf" "${NODEJS_DOWNLOAD_FILE}" OUTPUT_QUIET)
endif()

# Compile node as a shared library if needed
if(NOT EXISTS "${NODEJS_COMPILE_PATH}")
	if(WIN32)
		message(STATUS "Build NodeJS shared library")

		if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
			set(NODEJS_COMPILE_ARCH "x86")
		else()
			set(NODEJS_COMPILE_ARCH "x64")
		endif()

		# Check vs2017 or vs2015
		if(MSVC_VERSION EQUAL 1900)
			set(NODEJS_MSVC_VER vs2015)
		elseif(MSVC_VERSION GREATER 1900)
			set(NODEJS_MSVC_VER vs2017)
		endif()

		if("${CMAKE_BUILD_TYPE}" EQUAL "Debug")
			execute_process(COMMAND vcbuild.bat dll debug ${NODEJS_COMPILE_ARCH} ${NODEJS_MSVC_VER} WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}" OUTPUT_QUIET)
		else()
			execute_process(COMMAND vcbuild.bat dll release ${NODEJS_COMPILE_ARCH} ${NODEJS_MSVC_VER} WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}" OUTPUT_QUIET)
		endif()

		# Copy library to MetaCall output path
		file(COPY ${NODEJS_OUTPUT_PATH}/node.dll DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/node.dll)
	else()
		message(STATUS "Configure NodeJS shared library")

		if("${CMAKE_BUILD_TYPE}" EQUAL "Debug")
			execute_process(COMMAND sh ./configure --shared --debug WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}" OUTPUT_QUIET)
		else()
			execute_process(COMMAND sh ./configure --shared WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}" OUTPUT_QUIET)
		endif()

		message(STATUS "Build NodeJS shared library")

		include(ProcessorCount)

		ProcessorCount(N)

		if(NOT N EQUAL 0)
			execute_process(COMMAND make -j${N} -C out BUILDTYPE=${CMAKE_BUILD_TYPE} V=1 WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}" OUTPUT_QUIET)
		else()
			execute_process(COMMAND make -C out BUILDTYPE=${CMAKE_BUILD_TYPE} V=1 WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}" OUTPUT_QUIET)
		endif()
	endif()
endif()

# Find library
find_library(NODEJS_LIBRARY
	NAMES ${NODEJS_LIBRARY_NAMES}
	PATHS ${NODEJS_LIBRARY_PATH}
	DOC "NodeJS JavaScript Runtime Library"
)

find_package_handle_standard_args(NODEJS
	REQUIRED_VARS NODEJS_EXECUTABLE NODEJS_INCLUDE_DIR NODEJS_LIBRARY
	VERSION_VAR NODEJS_VERSION
)

mark_as_advanced(NODEJS_EXECUTABLE NODEJS_INCLUDE_DIR NODEJS_LIBRARY)

if(NODEJS_FOUND)
	set(NODEJS_INCLUDE_DIRS ${NODEJS_INCLUDE_DIR})
endif()

if(_NODEJS_CMAKE_DEBUG)
	message(STATUS "NODEJS_INCLUDE_DIR: ${NODEJS_INCLUDE_DIR}")
	message(STATUS "NODEJS_VERSION: ${NODEJS_VERSION}")
	message(STATUS "NODEJS_UV_VERSION: ${NODEJS_UV_VERSION}")
	message(STATUS "NODEJS_V8_VERSION: ${NODEJS_V8_VERSION}")
	message(STATUS "NODEJS_V8_VERSION_HEX: ${NODEJS_V8_VERSION_HEX}")
	message(STATUS "NODEJS_LIBRARY: ${NODEJS_LIBRARY}")
	message(STATUS "NODEJS_EXECUTABLE: ${NODEJS_EXECUTABLE}")
endif()
