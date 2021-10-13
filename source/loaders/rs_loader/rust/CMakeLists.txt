# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_RS)
	return()
endif()

#
# Plugin name and options
#

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/rust-toolchain TARGET_TOOLCHAIN)
string(STRIP "${TARGET_TOOLCHAIN}" TARGET_TOOLCHAIN)

find_package(Rust COMPONENTS ${TARGET_TOOLCHAIN})

if(NOT Rust_FOUND)
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

add_custom_target(${target}_runtime
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Rust_RUSTC_LIBRARIES} ${TARGET_OUTPUT_PATH}
)

add_custom_target(${target} ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CARGO_EXECUTABLE} build ${TARGET_BUILD_TYPE}
	# TODO: $ORIGIN does not work, but even using absolute path, the library librustc_driver depends on libstd and libLLVM
	# but they have the rpath hardcoded to the rustup folder, for mitigating this, we are using LD_LIBRARY_PATH in the test
	# although it may cause problems in the future in the distributable or docker builds, this must be reviewed
	# COMMAND ${CMAKE_COMMAND} -E env RUSTFLAGS='-C link-arg=-Wl,-rpath,${TARGET_OUTPUT_PATH}' ${CARGO_EXECUTABLE} build ${TARGET_BUILD_TYPE}
	# COMMAND ${CMAKE_COMMAND} -E env RUSTFLAGS='-C link-arg=-Wl,-rpath,\$ORIGIN' ${CARGO_EXECUTABLE} build ${TARGET_BUILD_TYPE}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TARGET_BUILD_PATH} ${TARGET_OUTPUT}
	DEPENDS ${target}_runtime
)

set_property(TARGET ${target}
	PROPERTY OUTPUT_NAME ${TARGET_OUTPUT}
)

#
# Deployment
#

# Library
install(FILES
	${TARGET_OUTPUT}
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)

# Rust Runtime (pack the runtime meanwhile Rust runtime is not distributed as a library)
install(FILES
	${Rust_RUSTC_LIBRARIES}
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
