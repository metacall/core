# TODO: Automatically call wat2wasm on scripts when changed

# Check if this script is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_WASM OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_WASM)
	return()
endif()

# Append cmake path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Wasm project utility
include(WasmProject)

add_subdirectory(tests)
