#
#	CMake Find Patchelf by Parra Studios
#	CMake script to find Patchelf executable.
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

if(Patchelf_FOUND)
	return()
endif()

if(WIN32 OR APPLE)
	message(WARNING "Patchelf not supported in MacOs or Windows")
endif()

set(Patchelf_PREFIX_DIR "${CMAKE_BINARY_DIR}/Patchelf")
set(Patchelf_SOURCE_DIR  "${Patchelf_PREFIX_DIR}/src/patchelf")
set(Patchelf_INSTALL_DIR "${Patchelf_PREFIX_DIR}/install/patchelf")
set(Patchelf_STAMP_DIR   "${Patchelf_PREFIX_DIR}/stamp/patchelf")
set(Patchelf_TMP_DIR     "${Patchelf_PREFIX_DIR}/tmp/patchelf")

include(ExternalProject)

set(Patchelf_ENV_COMMAND env CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} CXXFLAGS=-static)

ExternalProject_Add(Patchelf
	PREFIX						"${Patchelf_PREFIX_DIR}"
	URL							"http://nixos.org/releases/patchelf/patchelf-0.11/patchelf-0.11.tar.bz2"
	SOURCE_DIR					"${Patchelf_SOURCE_DIR}"
	BINARY_DIR					"${Patchelf_SOURCE_DIR}"
	INSTALL_DIR					"${Patchelf_INSTALL_DIR}"
	STAMP_DIR					"${Patchelf_STAMP_DIR}"
	TMP_DIR						"${Patchelf_TMP_DIR}"
	CONFIGURE_COMMAND			${Patchelf_ENV_COMMAND} ${Patchelf_SOURCE_DIR}/configure
	INSTALL_COMMAND				""
	LOG_DOWNLOAD				ON
	LOG_CONFIGURE				ON
	LOG_BUILD					ON
	LOG_INSTALL					ON
	DOWNLOAD_EXTRACT_TIMESTAMP	OFF
)

set(Patchelf_EXECUTABLE "${CMAKE_BINARY_DIR}/Patchelf/src/patchelf/src/patchelf")
set(Patchelf_FOUND TRUE)
