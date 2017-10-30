# Check if plugins are enabled
if(NOT OPTION_BUILD_PLUGINS)
	return()
endif()

# Define loader modules
set(LOADER_MODULE_NAMES
	version
	preprocessor
	environment
	format
	log
	adt
	reflect
	dynlink
	serial
	configuration
	loader
)

# Plugins options
option(OPTION_BUILD_PLUGINS_C "Build C Foreign Function Interface library plugin." OFF)
option(OPTION_BUILD_PLUGINS_CS "Build C# CoreCLR 1.0.1 Runtime plugin." OFF)
option(OPTION_BUILD_PLUGINS_JSM "Build JavaScript SpiderMonkey 4.8 plugin." OFF)
option(OPTION_BUILD_PLUGINS_JS "Build JavaScript V8 5.1+ plugin." OFF)
option(OPTION_BUILD_PLUGINS_NODE "Build NodeJS 7.6 JavaScript Runtime plugin." OFF)
option(OPTION_BUILD_PLUGINS_MOCK "Build mock loader plugin." ON)
option(OPTION_BUILD_PLUGINS_PY "Build Python 3.5 C API plugin." OFF)
option(OPTION_BUILD_PLUGINS_RB "Build Ruby 2.3 C API plugin." OFF)

# Plugin packages
add_subdirectory(c_loader) # Foreign Function Interface library
add_subdirectory(cs_loader) # CoreCLR 1.0.1 Runtime
add_subdirectory(jsm_loader) # Mozilla's SpiderMonkey 4.8 JavaScript Engine
add_subdirectory(js_loader) # Google's V8 5.1 JavaScript Engine
add_subdirectory(node_loader) # NodeJS 7.6 JavaScript Runtime
add_subdirectory(mock_loader) # Mock plugin without dependencies (core testing)
add_subdirectory(py_loader) # Python 3.5 C API
add_subdirectory(rb_loader) # Ruby 2.3 C API
