# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_RS)
	return()
endif()

#
# Plugin name and options
#

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/rust-toolchain TARGET_TOOLCHAIN)
string(STRIP "${TARGET_TOOLCHAIN}" TARGET_TOOLCHAIN)

# For checking availability of external tools like miri (in the future), use: https://rust-lang.github.io/rustup-components-history/
set(Rust_TOOLCHAIN_COMPONENT_LIST
	cargo
	clippy
	llvm-tools-preview
	rust-analysis
	rust-analyzer-preview
	rust-std
	rustc
	rustc-dev
	rustfmt
	rust-src
)

find_package(Rust COMPONENTS ${TARGET_TOOLCHAIN})

if(NOT Rust_FOUND)
	message(SEND_ERROR "Rust not found")
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

set(TARGET_OUTPUT ${PROJECT_OUTPUT_DIR}/${TARGET_OUTPUT_NAME})

add_custom_target(${target}_runtime
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Rust_RUSTC_LIBRARIES} ${PROJECT_OUTPUT_DIR}
)

# Darwin: allow undefined metacall symbols (resolved at runtime via dynamic_lookup)
# Linux: embed $ORIGIN rpath so sibling libraries resolve at load time
if(APPLE)
	set(CARGO_RUSTFLAGS "\"-C\", \"link-args=-Wl,-undefined,dynamic_lookup\"")
else()
	set(CARGO_RUSTFLAGS "\"-C\", \"link-args=-Wl,-rpath,$ORIGIN\"")
endif()

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/.cargo/config.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/.cargo/config"
	@ONLY
)

include(PatchRPath)

set(RUSTC_LIBS)
foreach(rustc_lib ${Rust_RUSTC_LIBRARIES})
	get_filename_component(rustc_lib_name "${rustc_lib}" NAME)
	list(APPEND RUSTC_LIBS "${PROJECT_OUTPUT_DIR}/${rustc_lib_name}")
endforeach()

# Patching runs after cargo build, not before
set(PATCH_LIBS "${TARGET_OUTPUT}" ${RUSTC_LIBS})
add_custom_target(${target}_rpath ALL DEPENDS ${target})
metacall_patch_rpath(${target}_rpath "${PATCH_LIBS}" "${PROJECT_OUTPUT_DIR}")

if(OPTION_BUILD_MUSL)
	set(RUST_CTR_STATIC "-Ctarget-feature=-crt-static -Clink-self-contained=off -L/usr/lib/x86_64-linux-musl") #-Clink-args=--dynamic-linker /lib/ld-musl-x86_64.so.1
	set(RUSTFLAGS ${CMAKE_COMMAND} -E env RUSTFLAGS=${RUST_CTR_STATIC})
else()
	set(RUSTFLAGS)
endif()

add_custom_target(${target} ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${RUSTFLAGS} ${Rust_CARGO_EXECUTABLE} build ${TARGET_BUILD_TYPE}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TARGET_BUILD_PATH} ${TARGET_OUTPUT}
	DEPENDS ${target}_runtime
)

set_property(TARGET ${target}
	PROPERTY OUTPUT_NAME ${TARGET_OUTPUT}
)

# Prepare the environment variable file for VSCode tests
include(Portability)

project_library_path(TEST_LIB_PATH
	${PROJECT_OUTPUT_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/.vscode/launch.json.in ${CMAKE_CURRENT_SOURCE_DIR}/.vscode/launch.json @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/.vscode/settings.json.in ${CMAKE_CURRENT_SOURCE_DIR}/.vscode/settings.json @ONLY)

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
	${RUSTC_LIBS}
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
