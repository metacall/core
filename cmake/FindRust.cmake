#
#	CMake Find Rust by Parra Studios
#	CMake script to find Rust compiler and tools.
#
#	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
# RUST_FOUND - True if rust was found
# CARGO_HOME - Cargo home folder
# CARGO_EXECUTABLE - Cargo package manager executable path
# RUSTC_EXECUTABLE - Rust compiler executable path
# RUSTDOC_EXECUTABLE - Rust doc executable plath
# RUSTUP_EXECUTABLE - Rustup executable path
# RUST_GDB_EXECUTABLE - Rust GDB debugger executable path
# RUST_LLDB_EXECUTABLE - Rust LLDB debugger executable path

if(WIN32)
	set(USER_HOME "$ENV{USERPROFILE}")
else()
	set(USER_HOME "$ENV{HOME}")
endif()

if(NOT DEFINED CARGO_HOME)
	if("$ENV{CARGO_HOME}" STREQUAL "")
		set(CARGO_HOME "${USER_HOME}/.cargo")
	else()
		set(CARGO_HOME "$ENV{CARGO_HOME}")
	endif()
endif()

set(RUST_PATHS
	/usr
	/usr/local
	${CARGO_HOME}
)

find_program(CARGO_EXECUTABLE cargo
	HINTS ${RUST_PATHS}
	PATH_SUFFIXES "bin"
)

find_program(RUSTC_EXECUTABLE rustc
	HINTS ${RUST_PATHS}
	PATH_SUFFIXES "bin"
)

find_program(RUSTDOC_EXECUTABLE rustdoc
	HINTS ${RUST_PATHS}
	PATH_SUFFIXES "bin"
)

find_program(RUSTUP_EXECUTABLE rustup
	HINTS ${RUST_PATHS}
	PATH_SUFFIXES "bin"
)

find_program(RUST_GDB_EXECUTABLE rust-gdb
	HINTS ${RUST_PATHS}
	PATH_SUFFIXES "bin"
)

find_program(RUST_LLDB_EXECUTABLE rust-lldb
	HINTS ${RUST_PATHS}
	PATH_SUFFIXES "bin"
)

if(CARGO_EXECUTABLE AND RUSTC_EXECUTABLE AND RUSTDOC_EXECUTABLE)
	set(CARGO_HOME "${CARGO_HOME}" CACHE PATH "Rust Cargo Home")
	execute_process(
		COMMAND ${RUSTC_EXECUTABLE} --version
		OUTPUT_VARIABLE RUSTC_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	string(REGEX REPLACE "rustc ([^ ]+) .*" "\\1" RUSTC_VERSION "${RUSTC_VERSION}")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Rust
	FOUND_VAR RUST_FOUND
	REQUIRED_VARS CARGO_EXECUTABLE RUSTC_EXECUTABLE
	VERSION_VAR RUSTC_VERSION
)

mark_as_advanced(
	RUST_FOUND
	CARGO_EXECUTABLE
	RUSTC_EXECUTABLE
	RUSTUP_EXECUTABLE
	RUSTDOC_EXECUTABLE
	RUST_GDB_EXECUTABLE
	RUST_LLDB_EXECUTABLE
)
