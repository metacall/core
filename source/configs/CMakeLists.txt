# Check if configurations are enabled
if(NOT OPTION_BUILD_CONFIGS)
	return()
endif()

# Configuration options
option(OPTION_BUILD_CONFIGS_RAPID_JSON "Build RapidJSON library configuration." ON)

# Define configuration modules
set(CONFIGURATION_MODULE_NAMES
	version
	preprocessor
	format
	log
	adt
	dynlink
	reflect
	configuration
)

# Configuration packages
add_subdirectory(rapid_json_config) # RapidJSON library
