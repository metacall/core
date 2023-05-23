#
#	CMake Find Rust by Parra Studios
#	CMake script to find Rust compiler and tools.
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

# Find Rust executables and paths
#
# Rust_FOUND - True if rust was found
# Rust_CARGO_HOME - Cargo home folder
# Rust_CARGO_EXECUTABLE - Cargo package manager executable path
# Rust_RUSTC_EXECUTABLE - Rust compiler executable path
# Rust_RUSTC_VERSION - Rust compiler vesion string
# Rust_RUSTC_SYSROOT - Rust compiler root location (includes binaries and libraries)
# Rust_RUSTC_LIBRARIES - Rust compiler runtime library list
# Rust_RUSTDOC_EXECUTABLE - Rust doc executable plath
# Rust_RUSTUP_EXECUTABLE - Rustup executable path
# Rust_GDB_EXECUTABLE - Rust GDB debugger executable path
# Rust_LLDB_EXECUTABLE - Rust LLDB debugger executable path
# Rust_TOOLCHAIN_TRIPLET - Current triplet used in rust compiler

# Options
#
# Rust_CMAKE_DEBUG - Print the debug information and all constants values

# Example for fixing the toolchain
#
# find_package(Rust COMPONENTS nightly-2021-10-09)

option(Rust_CMAKE_DEBUG "Show full output of the Rust related commands for debugging." OFF)

if(WIN32)
	set(USER_HOME "$ENV{USERPROFILE}")
else()
	set(USER_HOME "$ENV{HOME}")
endif()

if(NOT DEFINED Rust_CARGO_HOME)
	if("$ENV{Rust_CARGO_HOME}" STREQUAL "")
		set(Rust_CARGO_HOME "${USER_HOME}/.cargo")
	else()
		set(Rust_CARGO_HOME "$ENV{Rust_CARGO_HOME}")
	endif()
endif()

set(Rust_CARGO_HOME "${Rust_CARGO_HOME}" CACHE PATH "Rust Cargo Home")

set(Rust_PATHS
	/usr
	/usr/local
	${Rust_CARGO_HOME}
)

find_program(Rust_RUSTUP_EXECUTABLE rustup
	HINTS ${Rust_PATHS}
	PATH_SUFFIXES "bin"
)

if(Rust_RUSTUP_EXECUTABLE AND Rust_FIND_COMPONENTS)
	# Install the required toolchain (only one allowed by now)
	list(GET Rust_FIND_COMPONENTS 0 Rust_TOOLCHAIN)

	if(Rust_TOOLCHAIN)
		execute_process(
			COMMAND ${Rust_RUSTUP_EXECUTABLE} toolchain install ${Rust_TOOLCHAIN} --force
			OUTPUT_VARIABLE Rust_OUTPUT
			ERROR_VARIABLE Rust_OUTPUT
			RESULT_VARIABLE Rust_STATUS
		)

		if(Rust_CMAKE_DEBUG)
			message(STATUS "${Rust_OUTPUT}")
		else()
			if(Rust_STATUS AND NOT Rust_STATUS EQUAL 0)
				message(FATAL_ERROR "${Rust_OUTPUT}")
			endif()
		endif()

		execute_process(
			COMMAND ${Rust_RUSTUP_EXECUTABLE} default ${Rust_TOOLCHAIN}
			OUTPUT_VARIABLE Rust_OUTPUT
			ERROR_VARIABLE Rust_OUTPUT
			RESULT_VARIABLE Rust_STATUS
		)

		if(Rust_CMAKE_DEBUG)
			message(STATUS "${Rust_OUTPUT}")
		else()
			if(Rust_STATUS AND NOT Rust_STATUS EQUAL 0)
				message(FATAL_ERROR "${Rust_OUTPUT}")
			endif()
		endif()

		foreach(Rust_TOOLCHAIN_COMPONENT ${Rust_TOOLCHAIN_COMPONENT_LIST})
			execute_process(
				COMMAND ${Rust_RUSTUP_EXECUTABLE} toolchain install ${Rust_TOOLCHAIN} --component ${Rust_TOOLCHAIN_COMPONENT}
				OUTPUT_VARIABLE Rust_OUTPUT
				ERROR_VARIABLE Rust_OUTPUT
				RESULT_VARIABLE Rust_STATUS
			)

			if(Rust_CMAKE_DEBUG)
				message(STATUS "${Rust_OUTPUT}")
			else()
				if(Rust_STATUS AND NOT Rust_STATUS EQUAL 0)
					message(FATAL_ERROR "${Rust_OUTPUT}")
				endif()
			endif()
		endforeach()

		# Obtain toolchain full name and triplet
		execute_process(
			COMMAND ${Rust_RUSTUP_EXECUTABLE} default
			OUTPUT_VARIABLE Rust_TOOLCHAIN_FULL_NAME
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		string(REPLACE " " ";" Rust_TOOLCHAIN_FULL_NAME ${Rust_TOOLCHAIN_FULL_NAME})
		list(GET Rust_TOOLCHAIN_FULL_NAME 0 Rust_TOOLCHAIN_FULL_NAME)
		string(REPLACE "${Rust_TOOLCHAIN}-" "" Rust_TOOLCHAIN_TRIPLET ${Rust_TOOLCHAIN_FULL_NAME})
	endif()

endif()

find_program(Rust_CARGO_EXECUTABLE cargo
	HINTS ${Rust_PATHS}
	PATH_SUFFIXES "bin"
)

find_program(Rust_RUSTC_EXECUTABLE rustc
	HINTS ${Rust_PATHS}
	PATH_SUFFIXES "bin"
)

find_program(Rust_RUSTDOC_EXECUTABLE rustdoc
	HINTS ${Rust_PATHS}
	PATH_SUFFIXES "bin"
)

find_program(Rust_GDB_EXECUTABLE rust-gdb
	HINTS ${Rust_PATHS}
	PATH_SUFFIXES "bin"
)

find_program(Rust_LLDB_EXECUTABLE rust-lldb
	HINTS ${Rust_PATHS}
	PATH_SUFFIXES "bin"
)

if(Rust_RUSTC_EXECUTABLE)
	execute_process(
		COMMAND ${Rust_RUSTC_EXECUTABLE} --version
		OUTPUT_VARIABLE Rust_RUSTC_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	string(REGEX REPLACE "rustc ([^ ]+) .*" "\\1" Rust_RUSTC_VERSION "${Rust_RUSTC_VERSION}")

	execute_process(
		COMMAND ${Rust_RUSTC_EXECUTABLE} --print sysroot
		OUTPUT_VARIABLE Rust_RUSTC_SYSROOT
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	file(
		GLOB Rust_RUSTC_LIBRARIES
		${Rust_RUSTC_SYSROOT}/lib/*${CMAKE_SHARED_LIBRARY_SUFFIX}
	)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Rust
	FOUND_VAR Rust_FOUND
	REQUIRED_VARS Rust_CARGO_EXECUTABLE Rust_RUSTC_EXECUTABLE Rust_RUSTC_LIBRARIES
	VERSION_VAR Rust_RUSTC_VERSION
)

mark_as_advanced(
	Rust_FOUND
	Rust_CARGO_EXECUTABLE
	Rust_RUSTC_EXECUTABLE
	Rust_RUSTC_SYSROOT
	Rust_RUSTC_LIBRARIES
	Rust_RUSTUP_EXECUTABLE
	Rust_RUSTDOC_EXECUTABLE
	Rust_GDB_EXECUTABLE
	Rust_LLDB_EXECUTABLE
)

if(Rust_CMAKE_DEBUG)
	message(STATUS "Rust_CARGO_EXECUTABLE: ${Rust_CARGO_EXECUTABLE}")
	message(STATUS "Rust_RUSTC_EXECUTABLE: ${Rust_RUSTC_EXECUTABLE}")
	message(STATUS "Rust_RUSTC_SYSROOT: ${Rust_RUSTC_SYSROOT}")
	message(STATUS "Rust_RUSTC_LIBRARIES: ${Rust_RUSTC_LIBRARIES}")
	message(STATUS "Rust_RUSTUP_EXECUTABLE: ${Rust_RUSTUP_EXECUTABLE}")
	message(STATUS "Rust_GDB_EXECUTABLE: ${Rust_GDB_EXECUTABLE}")
	message(STATUS "Rust_LLDB_EXECUTABLE: ${Rust_LLDB_EXECUTABLE}")
	message(STATUS "Rust_TOOLCHAIN_TRIPLET: ${Rust_TOOLCHAIN_TRIPLET}")
endif()
