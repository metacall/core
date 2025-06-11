# Check if loaders are enabled
if(NOT OPTION_BUILD_LOADERS)
	return()
endif()

# Plugins options
option(OPTION_BUILD_LOADERS_C "Build C Foreign Function Interface library loader plugin." OFF)
option(OPTION_BUILD_LOADERS_COB "Build GNU/COBOL 2.2 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_CR "Build Crystal 0.33.0 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_CS "Build C# CoreCLR 5.0.17 Runtime loader plugin." OFF)
option(OPTION_BUILD_LOADERS_DART "Build Dart VM 2.8.4 Runtime loader plugin." OFF)
option(OPTION_BUILD_LOADERS_EXT "Build MetaCall Core Extension loader plugin." ON)
option(OPTION_BUILD_LOADERS_FILE "Build File System loader plugin." OFF)
option(OPTION_BUILD_LOADERS_JAVA "Build Java Virtual Machine loader plugin." OFF)
option(OPTION_BUILD_LOADERS_JL "Build Julia 1.6 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_JSM "Build JavaScript SpiderMonkey 4.8 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_JS "Build JavaScript V8 5.1+ loader plugin." OFF)
option(OPTION_BUILD_LOADERS_LLVM "Build LLVM 11 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_LUA "Build LuaJIT2 v2.1 (OpenResty fork) loader plugin." OFF)
option(OPTION_BUILD_LOADERS_MOCK "Build mock loader loader plugin." ON)
option(OPTION_BUILD_LOADERS_NODE "Build NodeJS v12.21.0 JavaScript Runtime loader plugin." OFF)
option(OPTION_BUILD_LOADERS_PY "Build Python 3.9 C API loader plugin." OFF)
option(OPTION_BUILD_LOADERS_RB "Build Ruby 2.7 C API loader plugin." OFF)
option(OPTION_BUILD_LOADERS_RS "Build Rust 1.55.0 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_RPC "Build cURL Remote Procedure Call loader plugin." OFF)
option(OPTION_BUILD_LOADERS_TS "Build TypeScript 3.9.7 Runtime loader plugin." OFF)
option(OPTION_BUILD_LOADERS_WASM "Build WebAssembly Virtual Machine loader plugin." OFF)

#
# Configuration for loaders
#
# The following list of macros is used for generating a configuration file which includes
# dependencies, this is required because now loaders can either load the dependencies or
# use the existing dependencies if MetaCall is being loaded from python.exe, node.exe
# or similar... so we require to delay the loader dependencies until we know if
# their dependencies are already present.
#
# This set of macros provies a flexible way of defining the configuration and dependencies.
#
# 1) The most basic case, let's assume we have the dependencies in the system, like in Python:
#
#     loader_configuration_begin(py_loader)
#     loader_configuration_deps(python "${Python3_LIBRARY}")
#     loader_configuartion_end()
#
# 2) Maybe you want to have multiple paths for a library:
#
#     loader_configuration_begin(node_loader)
#     loader_configuration_deps(node "/path/to/libnode.so" "/alternative/path/to/libnode.so")
#     loader_configuartion_end()
#
# 3) Or the dependencies are compiled by you and you so you have different folders for
# for development (build folder) and for when installing it:
#
#     loader_configuration_begin(node_loader)
#     loader_configuration_deps(node "/path/to/build/folder/libnode.so")
#     loader_configuartion_end_development()
#
#     loader_configuration_begin(node_loader)
#     loader_configuration_deps(node "/path/to/install/folder/libnode.so")
#     loader_configuartion_end_install()
#
# 4) Or you have a custom template because your loader needs a configuration with more fields:
#
#     loader_configuration_begin(cs_loader)
#     loader_configuration_deps(netcore "/path/to/build/folder/libnetcore.so")
#     loader_configuartion_end()
#
# You can do any combination of those for defining the configuration of your loaders.
#

# Define loader template configuration
set(LOADER_CONFIGURATION_DEFAULT_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/loader.json.in")

# Define loader configuration for a specific loader
macro(loader_configuration_begin TARGET)
	set(LOADER_DEPENDENCIES "")
	set(OPTIONAL_TEMPLATE "${ARGV1}")

	# Optional argument for template
	if(NOT "${ARGV1}" STREQUAL "")
		set(LOADER_CONFIGURATION_TEMPLATE "${ARGV1}")
	else()
		set(LOADER_CONFIGURATION_TEMPLATE "${LOADER_CONFIGURATION_DEFAULT_TEMPLATE}")
	endif()

	set(LOADER_CONFIGURATION_TARGET "${TARGET}")
endmacro()

# Generate configuration with search paths for a loader
#
#	"search_paths": [ "C:/Program Files/ruby/bin/ruby_builtin_dlls" ],
#
macro(loader_configuration_paths LIBRARY_PATHS_LIST)
	if(NOT "${LIBRARY_PATHS_LIST}" STREQUAL "")
		# Normalize the paths
		string(REPLACE "\\" "/" LIBRARY_PATHS "${LIBRARY_PATHS_LIST}")

		# Convert to JSON
		string(REPLACE ";" "\", \"" LOADER_SEARCH_PATHS "\"${LIBRARY_PATHS}\"")
	else()
		set(LOADER_SEARCH_PATHS)
	endif()
endmacro()

# Generate configuration with dependencies for a loader
#
#	node_loader:
#		"node": [ "/path/to/libnode.so", "/alternative/path/to/libnode.so" ],
#
#	c_loader:
#		"libffi": [ ... ],
#		"libclang": [ ... ],
#		"libtcc": [ ... ]
macro(loader_configuration_deps LIBRARY)
	# Add new line from previous dependency
	if(NOT "${LOADER_DEPENDENCIES}" STREQUAL "")
		string(APPEND LOADER_DEPENDENCIES ",\n\t\t")
	endif()

	# Define the library
	string(APPEND LOADER_DEPENDENCIES "\"${LIBRARY}\": [")

	# Define the paths
	set(FIRST_ARGUMENT 0)
	foreach(DEPENDENCY IN ITEMS ${ARGN})
		if(${FIRST_ARGUMENT} EQUAL 0)
			# Set first path
			string(APPEND LOADER_DEPENDENCIES "\"${DEPENDENCY}\"")
			set(FIRST_ARGUMENT 1)
		else()
			# Set the rest of the paths
			string(APPEND LOADER_DEPENDENCIES ", \"${DEPENDENCY}\"")
		endif()
	endforeach()

	# Finalize the list
	string(APPEND LOADER_DEPENDENCIES "]")
endmacro()

# Commit development version
macro(loader_configuartion_end_development)
	if(NOT DEFINED LOADER_CONFIGURATION_TEMPLATE OR NOT DEFINED LOADER_CONFIGURATION_TARGET)
		return()
	endif()

	configure_file(${LOADER_CONFIGURATION_TEMPLATE} ${CONFIGURATION_DIR}/${LOADER_CONFIGURATION_TARGET}.json)

	unset(LOADER_DEPENDENCIES)
	unset(LOADER_CONFIGURATION_TEMPLATE)
	unset(LOADER_CONFIGURATION_TARGET)
endmacro()

# Commit install version
macro(loader_configuartion_end_install)
	if(NOT DEFINED LOADER_CONFIGURATION_TEMPLATE OR NOT DEFINED LOADER_CONFIGURATION_TARGET)
		return()
	endif()

	configure_file(${LOADER_CONFIGURATION_TEMPLATE} ${CONFIGURATION_DIR}/install/configurations/${LOADER_CONFIGURATION_TARGET}.json)

	unset(LOADER_DEPENDENCIES)
	unset(LOADER_CONFIGURATION_TEMPLATE)
	unset(LOADER_CONFIGURATION_TARGET)
endmacro()

# Commit both versions
macro(loader_configuartion_end)
	if(NOT DEFINED LOADER_CONFIGURATION_TEMPLATE OR NOT DEFINED LOADER_CONFIGURATION_TARGET)
		return()
	endif()

	configure_file(${LOADER_CONFIGURATION_TEMPLATE} ${CONFIGURATION_DIR}/${LOADER_CONFIGURATION_TARGET}.json)
	configure_file(${LOADER_CONFIGURATION_TEMPLATE} ${CONFIGURATION_DIR}/install/configurations/${LOADER_CONFIGURATION_TARGET}.json)

	unset(LOADER_DEPENDENCIES)
	unset(LOADER_CONFIGURATION_TEMPLATE)
	unset(LOADER_CONFIGURATION_TARGET)
endmacro()

# Plugin packages
add_subdirectory(c_loader) # Foreign Function Interface library
add_subdirectory(cob_loader) # GNU/Cobol 2.2 Runtime
add_subdirectory(cr_loader) # Crystal 0.33.0 Runtime
add_subdirectory(cs_loader) # CoreCLR 5.0.17 Runtime
add_subdirectory(dart_loader) # Dart VM 2.8.4 Runtime
add_subdirectory(ext_loader) # MetaCall Core Extensions
add_subdirectory(file_loader) # File System
add_subdirectory(java_loader) # Java Virtual Machine
add_subdirectory(jl_loader) # Julia Runtime
add_subdirectory(jsm_loader) # Mozilla's SpiderMonkey 4.8 JavaScript Engine
add_subdirectory(js_loader) # Google's V8 5.1 JavaScript Engine
add_subdirectory(llvm_loader) # LLVM Compiler Infrastructure
add_subdirectory(lua_loader) # LuaJIT2 v2.1 (OpenResty fork) Runtime
add_subdirectory(mock_loader) # Mock loader plugin without dependencies (core testing)
add_subdirectory(node_loader) # NodeJS v12.21.0 JavaScript Runtime
add_subdirectory(py_loader) # Python 3.9 C API
add_subdirectory(rb_loader) # Ruby 2.7 C API
add_subdirectory(rs_loader) # Rust 1.55.0
add_subdirectory(rpc_loader) # cURL Remote Procedure Call
add_subdirectory(ts_loader) # TypeScript 3.9.7
add_subdirectory(wasm_loader) # WebAssembly Virtual Machine

# Optionally enable loader dependencies utility (for tests)
macro(add_loader_dependencies TARGET)
	set(LOADERS_LIST)

	foreach(LOADER ${ARGN})
		# Loaders come in the form of: py_loader, node_loader, ...
		# Convert them into OPTION_BUILD_LOADERS_PY, OPTION_BUILD_LOADERS_NODE, ...
		string(REPLACE "_loader" "" LOADER_TAG "${LOADER}")
		string(TOUPPER "${LOADER_TAG}" LOADER_TAG)

		# Check if the loader is enabled
		if(${OPTION_BUILD_LOADERS_${LOADER_TAG}})
			set(LOADERS_LIST
				${LOADERS_LIST}
				${loader}
			)
		endif()
	endforeach()

	if(LOADERS_LIST)
		add_dependencies(${TARGET} ${LOADERS_LIST})
	endif()
endmacro()
