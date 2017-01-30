# Check if configurations are enabled
if(NOT OPTION_BUILD_CONFIGS)
	return()
endif()

# Configuration options
option(OPTION_BUILD_CONFIGS_RAPID_JSON "Build RapidJSON library configuration." ON)

# Configuration packages
add_subdirectory(rapid_json_config) # RapidJSON library
