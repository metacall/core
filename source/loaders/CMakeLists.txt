# Check if loaders are enabled
if(NOT OPTION_BUILD_LOADERS)
	return()
endif()

# Define loader modules
set(LOADER_MODULE_NAMES
	version
	preprocessor
	environment
	format
	log
	memory
	portability
	adt
	filesystem
	reflect
	dynlink
	serial
	configuration
	loader
)

# Plugins options
option(OPTION_BUILD_LOADERS_C "Build C Foreign Function Interface library loader plugin." OFF)
option(OPTION_BUILD_LOADERS_CR "Build Crystal 0.33.0 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_CS "Build C# CoreCLR 2.2.8 Runtime loader plugin." OFF)
option(OPTION_BUILD_LOADERS_FILE "Build File System loader plugin." OFF)
option(OPTION_BUILD_LOADERS_JSM "Build JavaScript SpiderMonkey 4.8 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_JS "Build JavaScript V8 5.1+ loader plugin." OFF)
option(OPTION_BUILD_LOADERS_MOCK "Build mock loader loader plugin." ON)
option(OPTION_BUILD_LOADERS_NODE "Build NodeJS 10.16.3 JavaScript Runtime loader plugin." OFF)
option(OPTION_BUILD_LOADERS_PY "Build Python 3.7 C API loader plugin." OFF)
option(OPTION_BUILD_LOADERS_RB "Build Ruby 2.5 C API loader plugin." OFF)
option(OPTION_BUILD_LOADERS_RPC "Build cURL Remote Procedure Call loader plugin." OFF)
option(OPTION_BUILD_LOADERS_WASM "Build WebAssembly Virtual Machine loader plugin." OFF)

# Plugin packages
add_subdirectory(c_loader) # Foreign Function Interface library
add_subdirectory(cr_loader) # WebAssembly Virtual Machine
add_subdirectory(cs_loader) # CoreCLR 2.2.8 Runtime
add_subdirectory(jsm_loader) # Mozilla's SpiderMonkey 4.8 JavaScript Engine
add_subdirectory(js_loader) # Google's V8 5.1 JavaScript Engine
add_subdirectory(node_loader) # NodeJS 10.16.3 JavaScript Runtime
add_subdirectory(mock_loader) # Mock loader plugin without dependencies (core testing)
add_subdirectory(py_loader) # Python 3.7 C API
add_subdirectory(rb_loader) # Ruby 2.5 C API
add_subdirectory(file_loader) # File System
add_subdirectory(rpc_loader) # cURL Remote Procedure Call
add_subdirectory(wasm_loader) # WebAssembly Virtual Machine
