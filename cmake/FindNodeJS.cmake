#
# CMake Find NodeJS JavaScript Runtime by Parra Studios
# Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Find NodeJS executable and include paths
#
# NODEJS_FOUND - True if NodeJS was found
# NODEJS_INCLUDE_DIRS - NodeJS headers paths
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
#
# Configuration variables:
#
# NODEJS_CMAKE_DEBUG - Print paths for debugging
# NODEJS_EXECUTABLE_ONLY - Find only NodeJS executable (avoid library and include files)

# Prevent vervosity if already included
if(NODEJS_EXECUTABLE)
	set(NODEJS_FIND_QUIETLY TRUE)
endif()

option(NODEJS_CMAKE_DEBUG "Print paths for debugging NodeJS dependencies." OFF)
option(NODEJS_SHARED_UV "If it is enabled, libuv won't be required by this script." OFF)
option(NODEJS_BUILD_FROM_SOURCE "If it is enabled, NodeJS runtime library will be built from source." OFF)

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
)

# Find NodeJS include directories
if(MSVC OR CMAKE_BUILD_TYPE EQUAL "Debug")
	set(NODEJS_V8_HEADERS v8.h v8-version.h v8-profiler.h) # v8-debug.h
else()
	set(NODEJS_V8_HEADERS v8.h v8-version.h)
endif()

if(NOT NODEJS_SHARED_UV)
	set(NODEJS_UV_HEADERS uv.h)
endif()

set(NODEJS_HEADERS
	node.h
	node_api.h
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
	src
)

set(NODEJS_INCLUDE_PATHS
	/usr
	/usr/local
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

	# Check if NodeJS executable only is requested
	if(NODEJS_EXECUTABLE_ONLY)
		find_package_handle_standard_args(NODEJS
			REQUIRED_VARS NODEJS_EXECUTABLE
			VERSION_VAR NODEJS_VERSION
		)

		mark_as_advanced(NODEJS_EXECUTABLE)

		if(NODEJS_CMAKE_DEBUG)
			message(STATUS "NODEJS_VERSION: ${NODEJS_VERSION}")
			message(STATUS "NODEJS_EXECUTABLE: ${NODEJS_EXECUTABLE}")
		endif()

		return()
	endif()
endif()

# Find NodeJS includes
find_path(NODEJS_INCLUDE_DIR
	NAMES ${NODEJS_HEADERS}
	PATHS ${NODEJS_INCLUDE_PATHS}
	PATH_SUFFIXES ${NODEJS_INCLUDE_SUFFIXES}
	DOC "NodeJS JavaScript Runtime Headers"
)

# Check if the include directory contains all headers in the same folder
if(NODEJS_INCLUDE_DIR)
	foreach(HEADER IN ITEMS ${NODEJS_HEADERS})
		if(NOT EXISTS ${NODEJS_INCLUDE_DIR}/${HEADER})
			message(WARNING "NodeJS header ${HEADER} not found in ${NODEJS_INCLUDE_DIR}")
			set(NODEJS_INCLUDE_DIR FALSE)
			break()
		endif()
	endforeach()
endif()

# Find NodeJS V8 includes
find_path(NODEJS_V8_INCLUDE_DIR
	NAMES ${NODEJS_V8_HEADERS}
	PATHS ${NODEJS_INCLUDE_PATHS}
	PATH_SUFFIXES ${NODEJS_INCLUDE_SUFFIXES}
	DOC "NodeJS JavaScript Runtime V8 Headers"
)

# Check if the include directory contains all headers in the same folder
if(NODEJS_V8_INCLUDE_DIR)
	foreach(HEADER IN ITEMS ${NODEJS_V8_HEADERS})
		if(NOT EXISTS ${NODEJS_V8_INCLUDE_DIR}/${HEADER})
			message(WARNING "NodeJS header ${HEADER} not found in ${NODEJS_V8_INCLUDE_DIR}")
			set(NODEJS_V8_INCLUDE_DIR FALSE)
			break()
		endif()
	endforeach()
endif()

# Find NodeJS UV includes
find_path(NODEJS_UV_INCLUDE_DIR
	NAMES ${NODEJS_UV_HEADERS}
	PATHS ${NODEJS_INCLUDE_PATHS}
	PATH_SUFFIXES ${NODEJS_INCLUDE_SUFFIXES}
	DOC "NodeJS JavaScript Runtime UV Headers"
)

# Check if the include directory contains all headers in the same folder
if(NODEJS_UV_INCLUDE_DIR)
	foreach(HEADER IN ITEMS ${NODEJS_UV_HEADERS})
		if(NOT EXISTS ${NODEJS_UV_INCLUDE_DIR}/${HEADER})
			message(WARNING "NodeJS header ${HEADER} not found in ${NODEJS_UV_INCLUDE_DIR}")
			set(NODEJS_UV_INCLUDE_DIR FALSE)
			break()
		endif()
	endforeach()
endif()

# Download includes in case they are not distributed
if(NOT NODEJS_INCLUDE_DIR OR NOT NODEJS_V8_INCLUDE_DIR OR NOT NODEJS_UV_INCLUDE_DIR)
	if(NOT NODEJS_VERSION)
		# We do not have any way to know what version to install
		message(WARNING "NodeJS headers could not be found, neither a valid NodeJS version.")
		return()
	endif()

	# NodeJS download and output path (workaround for NodeJS headers)
	set(NODEJS_DOWNLOAD_URL "https://nodejs.org/dist/v${NODEJS_VERSION}/node-v${NODEJS_VERSION}-headers.tar.gz")
	set(NODEJS_BASE_PATH "${CMAKE_CURRENT_BINARY_DIR}/headers")
	set(NODEJS_DOWNLOAD_FILE "${NODEJS_BASE_PATH}/node-v${NODEJS_VERSION}-headers.tar.gz")
	set(NODEJS_OUTPUT_PATH "${NODEJS_BASE_PATH}/node-v${NODEJS_VERSION}")

	# Download node if needed
	if(NOT EXISTS "${NODEJS_DOWNLOAD_FILE}")
		message(STATUS "Downloading NodeJS headers")
		file(DOWNLOAD ${NODEJS_DOWNLOAD_URL} ${NODEJS_DOWNLOAD_FILE} SHOW_PROGRESS)
	endif()

	# Decompress node if needed
	if(NOT EXISTS "${NODEJS_OUTPUT_PATH}")
		message(STATUS "Extract NodeJS headers")
		execute_process(COMMAND ${CMAKE_COMMAND} -E tar "xvf" "${NODEJS_DOWNLOAD_FILE}" WORKING_DIRECTORY "${NODEJS_BASE_PATH}" OUTPUT_QUIET)
	endif()

	# Find NodeJS includes
	find_path(NODEJS_INCLUDE_DIR
		NAMES ${NODEJS_HEADERS}
		PATHS ${NODEJS_OUTPUT_PATH}
		PATH_SUFFIXES ${NODEJS_INCLUDE_SUFFIXES}
		DOC "NodeJS JavaScript Runtime Headers"
	)
endif()

if(NODEJS_INCLUDE_DIR)
	# Get node module version
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

if(NODEJS_V8_INCLUDE_DIR)
	# Detect NodeJS V8 version
	find_file(NODEJS_V8_VERSION_FILE_PATH v8-version.h
		PATHS ${NODEJS_V8_INCLUDE_DIR}
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
endif()

# Find NodeJS library from module version
if(NODEJS_MODULE_VERSION AND NOT NODEJS_BUILD_FROM_SOURCE)
	# NodeJS library names
	set(NODEJS_LIBRARY_NAMES
		libnode.so.${NODEJS_MODULE_VERSION}
		libnode.${NODEJS_MODULE_VERSION}.dylib
		libnode.${NODEJS_MODULE_VERSION}.dll
		node.lib
	)

	message(STATUS "Searching NodeJS library version ${NODEJS_MODULE_VERSION}")

	if(WIN32)
		set(NODEJS_COMPILE_PATH "${NODEJS_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}")
	else()
		set(NODEJS_COMPILE_PATH "${NODEJS_OUTPUT_PATH}/out/${CMAKE_BUILD_TYPE}")
	endif()

	if(WIN32)
		set(NODEJS_LIBRARY_PATH "${NODEJS_COMPILE_PATH}") # TODO: Set a valid install path
	else()
		set(NODEJS_LIBRARY_PATH "/usr/local/lib")
	endif()

	set(NODEJS_SYSTEM_LIBRARY_PATH "/lib/x86_64-linux-gnu" "/usr/lib/x86_64-linux-gnu") # TODO: Add others

	# Find library
	find_library(NODEJS_LIBRARY
		NAMES ${NODEJS_LIBRARY_NAMES}
		PATHS ${NODEJS_COMPILE_PATH} ${NODEJS_LIBRARY_PATH} ${NODEJS_SYSTEM_LIBRARY_PATH}
		DOC "NodeJS JavaScript Runtime Library"
	)

	if(NODEJS_LIBRARY)
		message(STATUS "NodeJS Library Found")
	endif()
endif()

# Install NodeJS library in case it is not distributed
if(NOT NODEJS_LIBRARY)
	message(STATUS "NodeJS library not found, trying to build it from source")

	# NodeJS download and output path (workaround to compile node as a shared library)
	set(NODEJS_DOWNLOAD_URL "https://nodejs.org/dist/v${NODEJS_VERSION}/node-v${NODEJS_VERSION}.tar.gz")
	set(NODEJS_BASE_PATH "${CMAKE_CURRENT_BINARY_DIR}/sources")
	set(NODEJS_DOWNLOAD_FILE "${NODEJS_BASE_PATH}/node-v${NODEJS_VERSION}.tar.gz")
	set(NODEJS_OUTPUT_PATH "${NODEJS_BASE_PATH}/node-v${NODEJS_VERSION}")

	# Download node if needed
	if(NOT EXISTS "${NODEJS_DOWNLOAD_FILE}")
		message(STATUS "Downloading NodeJS distribution v${NODEJS_VERSION}")
		file(DOWNLOAD ${NODEJS_DOWNLOAD_URL} ${NODEJS_DOWNLOAD_FILE} SHOW_PROGRESS)
	endif()

	# Decompress node if needed
	if(NOT EXISTS "${NODEJS_OUTPUT_PATH}")
		message(STATUS "Extract NodeJS distribution")
		execute_process(COMMAND ${CMAKE_COMMAND} -E tar "xvf" "${NODEJS_DOWNLOAD_FILE}" WORKING_DIRECTORY "${NODEJS_BASE_PATH}" OUTPUT_QUIET)
	endif()

	# Compile node as a shared library if needed
	if(NOT EXISTS "${NODEJS_COMPILE_PATH}")
		if(WIN32)
			if(NOT EXISTS "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/node.dll")
				message(STATUS "Build NodeJS shared library")

				if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
					set(NODEJS_COMPILE_ARCH "x86")
				else()
					set(NODEJS_COMPILE_ARCH "x64")
				endif()

				# Check vs2017 or vs2015 (TODO: Add more versions if they are supported by NodeJS)
				if(MSVC_VERSION EQUAL 1900)
					set(NODEJS_MSVC_VER vs2015)
				elseif(MSVC_VERSION GREATER 1900)
					set(NODEJS_MSVC_VER vs2017)
				endif()

				if(CMAKE_BUILD_TYPE STREQUAL "Debug")
					set(NODEJS_BUILD_TYPE debug)
				else()
					set(NODEJS_BUILD_TYPE release)
				endif()

				execute_process(COMMAND vcbuild.bat dll ${NODEJS_BUILD_TYPE} ${NODEJS_COMPILE_ARCH} ${NODEJS_MSVC_VER} WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}")

				# TODO: Implement msi build
				# execute_process(COMMAND vcbuild.bat dll ${NODEJS_BUILD_TYPE} ${NODEJS_COMPILE_ARCH} ${NODEJS_MSVC_VER} msi WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}")

				# Copy library to MetaCall output path
				file(COPY ${NODEJS_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}/node.dll DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/node.dll)

				message(STATUS "Install NodeJS shared library")

				# TODO: Implement install command
				#execute_process(COMMAND msiexec /a "node-v${NODEJS_VERSION}-${NODEJS_COMPILE_ARCH}.msi" WORKING_DIRECTORY "${NODEJS_COMPILE_PATH}")
			endif()

			# TODO: Delete this workaround after implementing the install command
			set(NODEJS_LIBRARY_PATH ${NODEJS_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
		else()
			message(STATUS "Configure NodeJS shared library")

			# Select the ICU library depending on the NodeJS version
			if("${NODEJS_VERSION_MAJOR}" GREATER_EQUAL "16")
				set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-69-1/icu4c-69_1-src.zip")
			elseif("${NODEJS_VERSION_MAJOR}" GREATER_EQUAL "15")
				set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-67-1/icu4c-67_1-src.zip")
			elseif("${NODEJS_VERSION_MAJOR}" GREATER_EQUAL "14")
				set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-66-1/icu4c-66_1-src.zip")
			elseif("${NODEJS_VERSION_MAJOR}" GREATER_EQUAL "12")
				set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-65-1/icu4c-65_1-src.zip")
			elseif("${NODEJS_VERSION_MAJOR}" GREATER_EQUAL "10")
				set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-64-2/icu4c-64_2-src.zip")
			endif()

			if("${CMAKE_BUILD_TYPE}" EQUAL "Debug")
				execute_process(COMMAND sh -c "./configure --with-icu-source=${ICU_URL} --shared --debug" WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}")
			else()
				execute_process(COMMAND sh -c "./configure --with-icu-source=${ICU_URL} --shared" WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}")
			endif()

			message(STATUS "Build NodeJS shared library")

			include(ProcessorCount)

			ProcessorCount(N)

			if(NOT N EQUAL 0)
				execute_process(COMMAND sh -c "alias python=`which python2.7`; make -j${N} -C out BUILDTYPE=${CMAKE_BUILD_TYPE} V=1" WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}")
			else()
				execute_process(COMMAND sh -c "alias python=`which python2.7`; make -C out BUILDTYPE=${CMAKE_BUILD_TYPE} V=1" WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}")
			endif()

			message(STATUS "Install NodeJS shared library")

			execute_process(COMMAND sh -c "make install" WORKING_DIRECTORY "${NODEJS_OUTPUT_PATH}")
		endif()
	endif()

	if(NODEJS_MODULE_VERSION)
		# NodeJS library names
		set(NODEJS_LIBRARY_NAMES
			libnode.so.${NODEJS_MODULE_VERSION}
			libnode.${NODEJS_MODULE_VERSION}.dylib
			libnode.${NODEJS_MODULE_VERSION}.dll
			node.lib
		)

		# Find library
		find_library(NODEJS_LIBRARY
			NAMES ${NODEJS_LIBRARY_NAMES}
			PATHS ${NODEJS_LIBRARY_PATH}
			DOC "NodeJS JavaScript Runtime Library"
		)
	endif()

	if(NOT NODEJS_LIBRARY)
		message(SEND_ERROR "NodeJS library not found and it could not be built from source")
	endif()
endif()

set(NODEJS_INCLUDE_DIRS "${NODEJS_INCLUDE_DIR}")

if(NODEJS_V8_INCLUDE_DIR)
	set(NODEJS_INCLUDE_DIRS "${NODEJS_INCLUDE_DIRS}" "${NODEJS_V8_INCLUDE_DIR}")
endif()

if(NODEJS_UV_INCLUDE_DIR)
	set(NODEJS_INCLUDE_DIRS "${NODEJS_INCLUDE_DIRS}" "${NODEJS_UV_INCLUDE_DIR}")
endif()

find_package_handle_standard_args(NODEJS
	REQUIRED_VARS NODEJS_EXECUTABLE NODEJS_INCLUDE_DIRS NODEJS_LIBRARY
	VERSION_VAR NODEJS_VERSION
)

mark_as_advanced(NODEJS_EXECUTABLE NODEJS_INCLUDE_DIRS NODEJS_LIBRARY)

if(NODEJS_CMAKE_DEBUG)
	message(STATUS "NODEJS_INCLUDE_DIRS: ${NODEJS_INCLUDE_DIRS}")
	message(STATUS "NODEJS_VERSION: ${NODEJS_VERSION}")
	message(STATUS "NODEJS_UV_VERSION: ${NODEJS_UV_VERSION}")
	message(STATUS "NODEJS_V8_VERSION: ${NODEJS_V8_VERSION}")
	message(STATUS "NODEJS_V8_VERSION_HEX: ${NODEJS_V8_VERSION_HEX}")
	message(STATUS "NODEJS_LIBRARY: ${NODEJS_LIBRARY}")
	message(STATUS "NODEJS_EXECUTABLE: ${NODEJS_EXECUTABLE}")
endif()
