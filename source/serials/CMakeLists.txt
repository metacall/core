# Check if serials are enabled
if(NOT OPTION_BUILD_SERIALS)
	return()
endif()

# Serial options
option(OPTION_BUILD_SERIALS_METACALL "MetaCall Native Format library serial." ON)
option(OPTION_BUILD_SERIALS_RAPID_JSON "RapidJSON library serial." ON)

# Define serial modules
set(SERIAL_MODULE_NAMES
	version
	preprocessor
	environment
	format
	threading
	log
	memory
	portability
	adt
	reflect
	dynlink
	serial
)

set(SERIAL_MODULE_INCLUDES)

foreach(SERIAL_MODULE ${SERIAL_MODULE_NAMES})
	get_target_property(INCLUDES ${SERIAL_MODULE} INCLUDE_DIRECTORIES)
	list(APPEND SERIAL_MODULE_INCLUDES ${INCLUDES})
endforeach()

# Serial packages
add_subdirectory(metacall_serial) # MetaCall Native Format library
add_subdirectory(rapid_json_serial) # RapidJSON library
