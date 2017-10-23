# Check if serials are enabled
if(NOT OPTION_BUILD_SERIALS)
	return()
endif()

# Configuration options
option(OPTION_BUILD_SERIALS_RAPID_JSON "Build RapidJSON library serial." ON)

# Define serial modules
set(SERIAL_MODULE_NAMES
	version
	preprocessor
	environment
	format
	log
	adt
	dynlink
	reflect
	configuration
	serial
)

# Serial packages
add_subdirectory(rapid_json_serial) # RapidJSON library
