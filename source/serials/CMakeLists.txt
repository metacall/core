# Check if serials are enabled
if(NOT OPTION_BUILD_SERIALS)
	return()
endif()

# Serial options
option(OPTION_BUILD_SERIALS_METACALL   "MetaCall Native Format library serial." ON)
option(OPTION_BUILD_SERIALS_RAPID_JSON "RapidJSON library serial." ON)
option(OPTION_BUILD_SERIALS_GRPC       "gRPC serial." ON)

# Serial packages
add_subdirectory(metacall_serial)
add_subdirectory(rapid_json_serial)
add_subdirectory(grpc_serial)
