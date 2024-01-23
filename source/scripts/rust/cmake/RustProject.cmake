#
#	Rust project generator by Parra Studios
#	Generates a Rust project embedded into CMake.
#
#	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

if(RUSTPROJECT_FOUND)
	return()
endif()

set(RUSTPROJECT_FOUND YES)

#
# Generic script project generator
#

include(ScriptProject)

# Define current Rust project configuration path
get_filename_component(RUST_PROJECT_CONFIG_PATH ${CMAKE_CURRENT_LIST_FILE} PATH)

#
# Rust sub-project util function
#

function(rust_project target version)

	# TODO

	# Configuration
	set(PACKAGE_NAME		${target})
	set(PACKAGE_VERSION		${version})
	set(PACKAGE_OUTPUT		"${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

	# Create project file
	script_project(${target} rust ${RUST_PROJECT_CONFIG_PATH}/RustProject.cmake.in)

endfunction()

function(rust_package target version script)
	
	# Configuration
	set(PACKAGE_NAME		${target})
	set(PACKAGE_VERSION		${version})
	set(PACKAGE_OUTPUT		"${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

	set(configuration ${RUST_PROJECT_CONFIG_PATH}/RustProject.cmake.in)
	set(language "rust")
	# Define upper and lower versions of the language
	string(TOLOWER ${language} language_lower)

	# Define project target name
	set(custom_target "${language_lower}-${target}")

	# Define target for the configuration
	set(PACKAGE_TARGET "${custom_target}")

	# Create project file
	configure_file(${configuration} ${custom_target}-config.cmake @ONLY)

	# Set custom target
	add_custom_target(${custom_target} ALL)

	# Set project properties
	set_target_properties(${custom_target}
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS}
		FOLDER "${IDE_FOLDER}/${language}"
	)

	# Compile scripts
	add_custom_command(TARGET ${custom_target} PRE_BUILD
		# Fix the version of rustc
		COMMAND ${Rust_RUSTUP_EXECUTABLE} default nightly-2021-12-04
		COMMAND ${Rust_RUSTC_EXECUTABLE} --crate-type=lib ${CMAKE_CURRENT_SOURCE_DIR}/source/${script}.rs --out-dir ${PROJECT_OUTPUT_DIR}
		COMMAND ${Rust_RUSTC_EXECUTABLE} --crate-type=dylib -Cprefer-dynamic ${CMAKE_CURRENT_SOURCE_DIR}/source/${script}.rs --out-dir ${PROJECT_OUTPUT_DIR}
	)

	# Include generated project file
	include(${CMAKE_CURRENT_BINARY_DIR}/${custom_target}-config.cmake)


endfunction()

function(cargo_package target version)

	# Configuration
	set(PACKAGE_NAME		${target})
	set(PACKAGE_VERSION		${version})
	set(PACKAGE_OUTPUT		"${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

	set(configuration ${RUST_PROJECT_CONFIG_PATH}/RustProject.cmake.in)
	set(language "rust")
	# Define upper and lower versions of the language
	string(TOLOWER ${language} language_lower)

	# Define project target name
	set(custom_target "${language_lower}-${target}")

	# Define target for the configuration
	set(PACKAGE_TARGET "${custom_target}")

	# Create project file
	configure_file(${configuration} ${custom_target}-config.cmake @ONLY)

	# Set custom target
	add_custom_target(${custom_target} ALL)

	# Set project properties
	set_target_properties(${custom_target}
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS}
		FOLDER "${IDE_FOLDER}/${language}"
	)

	# Compile project
	add_custom_command(TARGET ${custom_target} PRE_BUILD
		# Fix the version of rustc
		COMMAND ${Rust_RUSTUP_EXECUTABLE} default nightly-2021-12-04
		COMMAND ${Rust_CARGO_EXECUTABLE} build --manifest-path ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml --target-dir ${PROJECT_OUTPUT_DIR}
	)

	# Include generated project file
	include(${CMAKE_CURRENT_BINARY_DIR}/${custom_target}-config.cmake)
endfunction()
