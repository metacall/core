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
option(OPTION_BUILD_LOADERS_CS "Build C# CoreCLR 1.0.1 Runtime loader plugin." OFF)
option(OPTION_BUILD_LOADERS_FILE "Build File System loader plugin." OFF)
option(OPTION_BUILD_LOADERS_JSM "Build JavaScript SpiderMonkey 4.8 loader plugin." OFF)
option(OPTION_BUILD_LOADERS_JS "Build JavaScript V8 5.1+ loader plugin." OFF)
option(OPTION_BUILD_LOADERS_MOCK "Build mock loader loader plugin." ON)
option(OPTION_BUILD_LOADERS_NODE "Build NodeJS 8.11.1 JavaScript Runtime loader plugin." OFF)
option(OPTION_BUILD_LOADERS_PY "Build Python 3.5 C API loader plugin." OFF)
option(OPTION_BUILD_LOADERS_RB "Build Ruby 2.3 C API loader plugin." OFF)
option(OPTION_BUILD_LOADERS_WASM "Build WebAssembly Virtual Machine loader plugin." OFF)

# Plugin packages
add_subdirectory(c_loader) # Foreign Function Interface library
add_subdirectory(cs_loader) # CoreCLR 1.0.1 Runtime
add_subdirectory(jsm_loader) # Mozilla's SpiderMonkey 4.8 JavaScript Engine
add_subdirectory(js_loader) # Google's V8 5.1 JavaScript Engine
add_subdirectory(node_loader) # NodeJS 8.11.1 JavaScript Runtime
add_subdirectory(mock_loader) # Mock loader plugin without dependencies (core testing)
add_subdirectory(py_loader) # Python 3.5 C API
add_subdirectory(rb_loader) # Ruby 2.3 C API
add_subdirectory(file_loader) # File System
add_subdirectory(wasm_loader) # WebAssembly Virtual Machine
