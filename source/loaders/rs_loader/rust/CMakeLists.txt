# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_RS)
	return()
endif()

#
# Plugin name and options
#

find_package(Rust)

if(NOT RUST_FOUND)
	message(STATUS "Rust not found")
	return()
endif()

# Target name
set(target rs_loader_impl)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target}")

set(TARGET_BUILD_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}${target}${CMAKE_SHARED_LIBRARY_SUFFIX}")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(TARGET_BUILD_TYPE "--release")
	set(TARGET_BUILD_PATH "${CMAKE_CURRENT_SOURCE_DIR}/target/release/${TARGET_BUILD_NAME}")
	set(TARGET_OUTPUT_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}${target}${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(TARGET_BUILD_TYPE)
	set(TARGET_BUILD_PATH "${CMAKE_CURRENT_SOURCE_DIR}/target/debug/${TARGET_BUILD_NAME}")
	set(TARGET_OUTPUT_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}${target}d${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()

if(MSVC)
	set(TARGET_OUTPUT_PATH "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
	set(TARGET_OUTPUT_PATH "${PROJECT_BINARY_DIR}")
endif()

set(TARGET_OUTPUT ${TARGET_OUTPUT_PATH}/${TARGET_OUTPUT_NAME})

add_custom_target(${target}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CARGO_EXECUTABLE} build ${TARGET_BUILD_TYPE}
	COMMAND ${CMAKE_COMMAND} -E copy ${TARGET_BUILD_PATH} ${TARGET_OUTPUT}
)

set_property(TARGET ${target}
	PROPERTY OUTPUT_NAME ${TARGET_OUTPUT}
)

#
# Deployment
#

# Files
install(FILES
	${TARGET_OUTPUT}
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
