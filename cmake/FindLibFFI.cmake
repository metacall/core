#
#	CMake Find Foreing Function Interface library by Parra Studios
#	CMake script to find FFI library.
#
#	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# Find FFI library and include paths
#
# LIBFFI_FOUND - True if FFI library was found
# LIBFFI_INCLUDE_DIRS - FFI headers path
# LIBFFI_LIBRARIES - List of FFI libraries
# LIBFFI_DEFINITIONS - FFI definitions

# Prevent vervosity if already included
if(LIBFFI_INCLUDE_DIRS)
	set(LIBFFI_FIND_QUITELY TRUE)
endif()

include(FindPackageHandleStandardArgs)

# Find package configuration module
find_package(PkgConfig)

# Find module
pkg_check_modules(PC_LIBFFI QUIET libffi)

# Find include path
find_path(LIBFFI_INCLUDE_DIR ffi.h HINTS ${PC_LIBFFI_INCLUDEDIR} ${PC_LIBFFI_INCLUDE_DIRS})

# Find library
find_library(LIBFFI_LIBRARY NAMES ffi HINTS ${PC_LIBFFI_LIBDIR} ${PC_LIBFFI_LIBRARY_DIRS})

# Define moudle variables
set(LIBFFI_DEFINITIONS ${PC_LIBFFI_CFLAGS_OTHER})
set(LIBFFI_LIBRARIES ${LIBFFI_LIBRARY})
set(LIBFFI_INCLUDE_DIRS ${LIBFFI_INCLUDE_DIR})

# Define FFI cmake module
find_package_handle_standard_args(LibFFI DEFAULT_MSG LIBFFI_LIBRARY LIBFFI_INCLUDE_DIR)

# Mark cmake module as advanced
mark_as_advanced(LIBFFI_INCLUDE_DIR LIBFFI_LIBRARY)
