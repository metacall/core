#
#	CMake Install Tiny C Compiler by Parra Studios
#	CMake script to install TCC library.
#
#	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# The following variables are set:
#
# LIBTCC_FOUND - True if TCC library was found
# LIBTCC_TARGET - Defines the TCC depends target (for using it as dependency on other targets)
# LIBTCC_INCLUDE_DIR - TCC headers path
# LIBTCC_LIBRARY - List of TCC libraries

if(LIBTCC_FOUND)
	return()
endif()

include(Portability)

set(LIBTCC_INSTALL_PREFIX "${PROJECT_OUTPUT_DIR}/libtcc")
file(MAKE_DIRECTORY ${LIBTCC_INSTALL_PREFIX})

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	set(LIBTCC_DEBUG "--debug")
else()
	set(LIBTCC_DEBUG)
endif()

# Configure
if(PROJECT_OS_FAMILY STREQUAL unix)
	set(LIBTCC_CONFIGURE ./configure --prefix=${LIBTCC_INSTALL_PREFIX} ${LIBTCC_DEBUG} --disable-static --with-libgcc --with-selinux)
elseif(PROJECT_OS_FAMILY STREQUAL win32)
	if(PROJECT_OS_NAME STREQUAL MinGW)
		set(LIBTCC_CONFIGURE ./configure --prefix=${LIBTCC_INSTALL_PREFIX} ${LIBTCC_DEBUG} --config-mingw32 --disable-static --with-libgcc --with-selinux)
	else()
		set(LIBTCC_CONFIGURE "")
	endif()
else()
	message(FATAL_ERROR "TCC library install support not implemented in this platform")
endif()

include(ProcessorCount)
ProcessorCount(N)

# Build
if(PROJECT_OS_BSD)
	set(LIBTCC_BUILD gmake -j${N})
elseif(PROJECT_OS_FAMILY STREQUAL unix)
	set(LIBTCC_BUILD make -j${N})
elseif(PROJECT_OS_FAMILY STREQUAL win32)
	if(PROJECT_OS_NAME STREQUAL MinGW)
		set(LIBTCC_BUILD make -j${N})
	else()
		set(LIBTCC_BUILD ./win32/build-tcc.bat -i ${LIBTCC_INSTALL_PREFIX})
	endif()
else()
	message(FATAL_ERROR "TCC library install support not implemented in this platform")
endif()

# Install
if(PROJECT_OS_BSD)
	set(LIBTCC_INSTALL gmake install)
elseif(PROJECT_OS_FAMILY STREQUAL win32 AND PROJECT_OS_NAME STREQUAL Windows)
	set(LIBTCC_INSTALL "")
else()
	set(LIBTCC_INSTALL make install)
endif()

set(LIBTCC_TARGET libtcc-depends)
set(LIBTCC_COMMIT_SHA "da11cf651576f94486dbd043dbfcde469e497574")
set(LIBTTC_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}tcc${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(LIBTTC_LIBRARY_PATH "${PROJECT_OUTPUT_DIR}/${LIBTTC_LIBRARY_NAME}")
set(LIBTTC_RUNTIME_PATH "${LIBTCC_INSTALL_PREFIX}/lib/tcc")
set(LIBTTC_RUNTIME_INCLUDE_PATH "${LIBTTC_RUNTIME_PATH}/include")
set(LIBTTC_RUNTIME_FILES
	"${LIBTTC_RUNTIME_PATH}/libtcc1.a"
	"${LIBTTC_RUNTIME_PATH}/bcheck.o"
	"${LIBTTC_RUNTIME_PATH}/bt-exe.o"
	"${LIBTTC_RUNTIME_PATH}/bt-log.o"
)

# LibTCC Proejct
ExternalProject_Add(${LIBTCC_TARGET}
	DOWNLOAD_NAME		tinycc.tar.gz
	URL					https://repo.or.cz/tinycc.git/snapshot/${LIBTCC_COMMIT_SHA}.tar.gz
	URL_MD5				cc0cde5f454fa3a8c068da95edaaea86
	CONFIGURE_COMMAND	${LIBTCC_CONFIGURE}
	BUILD_COMMAND		${LIBTCC_BUILD}
	BUILD_IN_SOURCE		true
	TEST_COMMAND		""
	UPDATE_COMMAND		""
	INSTALL_COMMAND		${LIBTCC_INSTALL}
	COMMAND				${CMAKE_COMMAND} -E copy "${LIBTCC_INSTALL_PREFIX}/lib/${LIBTTC_LIBRARY_NAME}" "${LIBTTC_LIBRARY_PATH}"
	COMMAND				${CMAKE_COMMAND} -E copy ${LIBTTC_RUNTIME_FILES} "${PROJECT_OUTPUT_DIR}"
)

# Install Library
install(FILES
	${LIBTTC_LIBRARY_PATH}
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)

# Runtime files
install(DIRECTORY
	${LIBTTC_RUNTIME_PATH}/
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
	FILES_MATCHING
	PATTERN "*.a"
	PATTERN "*.o"
	PATTERN "include" EXCLUDE
)

# Header files
install(DIRECTORY
	${LIBTTC_RUNTIME_INCLUDE_PATH}/
	DESTINATION ${INSTALL_INCLUDE}
	COMPONENT runtime
)

set(LIBTCC_INCLUDE_DIR	"${LIBTCC_INSTALL_PREFIX}/include")
set(LIBTCC_LIBRARY		"${LIBTTC_LIBRARY_PATH}")
set(LIBTCC_FOUND		TRUE)

mark_as_advanced(LIBTCC_INCLUDE_DIR LIBTCC_LIBRARY)

message(STATUS "Installing LibTCC ${LIBTCC_COMMIT_SHA}")
