# Check if this port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_RS)
	return()
endif()

#
# Port name and options
#

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/rust-toolchain TARGET_TOOLCHAIN)
string(STRIP "${TARGET_TOOLCHAIN}" TARGET_TOOLCHAIN)

find_package(Rust COMPONENTS ${TARGET_TOOLCHAIN})

if(NOT Rust_FOUND)
	message(STATUS "Rust not found")
	return()
endif()

# Target name
set(target rs_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

# Find bindgen
find_program(Rust_BINDGEN_EXECUTABLE bindgen
	HINTS ${Rust_CARGO_HOME}
	PATH_SUFFIXES "bin"
)

if(NOT Rust_BINDGEN_EXECUTABLE AND NOT OPTION_BUILD_GUIX)
	execute_process(COMMAND ${Rust_CARGO_EXECUTABLE} install bindgen)

	find_program(Rust_BINDGEN_EXECUTABLE bindgen
		HINTS ${Rust_CARGO_HOME}
		PATH_SUFFIXES "bin"
	)
endif()

# Generate bindings
if(Rust_BINDGEN_EXECUTABLE)
	add_custom_target(${target}_bindings
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ${Rust_BINDGEN_EXECUTABLE}
			--no-include-path-detection
			"${CMAKE_SOURCE_DIR}/source/metacall/include/metacall/metacall.h"
			-o "${CMAKE_CURRENT_SOURCE_DIR}/src/bindings.rs"
			--
			-I${CMAKE_SOURCE_DIR}/source/metacall/include
			-I${CMAKE_BINARY_DIR}/source/metacall/include
			-I${CMAKE_BINARY_DIR}/source/include
		DEPENDS ${META_PROJECT_NAME}::metacall
	)
endif()

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/bindings.rs" PROPERTIES GENERATED TRUE)

# Build Rust Port
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(TARGET_BUILD_TYPE "--release")
else()
	set(TARGET_BUILD_TYPE)
endif()

if(OPTION_BUILD_SANITIZER)
	set(SANITIZER_FLAGS
		CMAKE_ADDRESS_SANITIZER=1
	)
elseif(OPTION_BUILD_THREAD_SANITIZER)
	set(SANITIZER_FLAGS
		CMAKE_THREAD_SANITIZER=1
	)
else()
	set(SANITIZER_FLAGS)
endif()


add_custom_target(${target} ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} -E env CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} PROJECT_OUTPUT_DIR=${PROJECT_OUTPUT_DIR} ${SANITIZER_FLAGS}
		${Rust_CARGO_EXECUTABLE} build ${TARGET_BUILD_TYPE}
	DEPENDS ${META_PROJECT_NAME}::metacall
)

#
# Dependecies
#

if(Rust_BINDGEN_EXECUTABLE)
	add_dependencies(${target}
		${target}_bindings
	)
endif()

#
# Define test
#

if(OPTION_BUILD_THREAD_SANITIZER)
	# TODO: This test fails when run with thread sanitizers:
	#
	# WARNING: ThreadSanitizer: data race (pid=281586)
	# Read of size 8 at 0x7b9400000000 by main thread:
	# 	#0 memcpy ../../../../src/libsanitizer/sanitizer_common/sanitizer_common_interceptors.inc:827 (libtsan.so.0+0x6229e)
	# 	#1 memcpy ../../../../src/libsanitizer/sanitizer_common/sanitizer_common_interceptors.inc:819 (libtsan.so.0+0x6229e)
	# 	#2 core::mem::maybe_uninit::MaybeUninit$LT$T$GT$::assume_init::h2b1de5f8b96b1f2a library/core/src/mem/maybe_uninit.rs:627 (inlines_test-93e5a5076af88165+0x1934d)
	# 	#3 std::sync::mpmc::list::Channel$LT$T$GT$::read::h2b5457fbe04e9459 library/std/src/sync/mpmc/list.rs:374 (inlines_test-93e5a5076af88165+0x1934d)
	# 	#4 std::sync::mpmc::list::Channel$LT$T$GT$::recv::h9348078961d418d5 library/std/src/sync/mpmc/list.rs:423 (inlines_test-93e5a5076af88165+0x1934d)
	#
	# Previous write of size 8 at 0x7b9400000000 by thread T1:
	# 	#0 malloc ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:655 (libtsan.so.0+0x31c57)
	# 	#1 alloc::alloc::alloc::h3667a9b877dfbc28 library/alloc/src/alloc.rs:95 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#2 alloc::alloc::Global::alloc_impl::hdb29b2f8fe35cc4a library/alloc/src/alloc.rs:177 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#3 _$LT$alloc..alloc..Global$u20$as$u20$core..alloc..Allocator$GT$::allocate::h6fa64cb9b65ed2b1 library/alloc/src/alloc.rs:237 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#4 alloc::alloc::exchange_malloc::hb0a7f17b28ecb40b library/alloc/src/alloc.rs:326 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#5 alloc::boxed::Box$LT$T$GT$::new::h6bf549ae70c18f13 library/alloc/src/boxed.rs:219 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#6 std::sync::mpmc::list::Channel$LT$T$GT$::start_send::h55d2f0997a4120d9 library/std/src/sync/mpmc/list.rs:209 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#7 std::sync::mpmc::list::Channel$LT$T$GT$::send::hd0eaef9b8f56723e library/std/src/sync/mpmc/list.rs:402 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#8 std::sync::mpmc::Sender$LT$T$GT$::send::h859582461f6a4a5d library/std/src/sync/mpmc/mod.rs:128 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#9 std::sync::mpsc::Sender$LT$T$GT$::send::hd887dc2b5ce76a3a library/std/src/sync/mpsc/mod.rs:614 (inlines_test-93e5a5076af88165+0x1b27e)
	#
	# Location is heap block of size 8440 at 0x7b9400000000 allocated by thread T1:
	# 	#0 malloc ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:655 (libtsan.so.0+0x31c57)
	# 	#1 alloc::alloc::alloc::h3667a9b877dfbc28 library/alloc/src/alloc.rs:95 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#2 alloc::alloc::Global::alloc_impl::hdb29b2f8fe35cc4a library/alloc/src/alloc.rs:177 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#3 _$LT$alloc..alloc..Global$u20$as$u20$core..alloc..Allocator$GT$::allocate::h6fa64cb9b65ed2b1 library/alloc/src/alloc.rs:237 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#4 alloc::alloc::exchange_malloc::hb0a7f17b28ecb40b library/alloc/src/alloc.rs:326 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#5 alloc::boxed::Box$LT$T$GT$::new::h6bf549ae70c18f13 library/alloc/src/boxed.rs:219 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#6 std::sync::mpmc::list::Channel$LT$T$GT$::start_send::h55d2f0997a4120d9 library/std/src/sync/mpmc/list.rs:209 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#7 std::sync::mpmc::list::Channel$LT$T$GT$::send::hd0eaef9b8f56723e library/std/src/sync/mpmc/list.rs:402 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#8 std::sync::mpmc::Sender$LT$T$GT$::send::h859582461f6a4a5d library/std/src/sync/mpmc/mod.rs:128 (inlines_test-93e5a5076af88165+0x1b27e)
	# 	#9 std::sync::mpsc::Sender$LT$T$GT$::send::hd887dc2b5ce76a3a library/std/src/sync/mpsc/mod.rs:614 (inlines_test-93e5a5076af88165+0x1b27e)
	#
	# Thread T1 'inlines' (tid=281588, finished) created by main thread at:
	# 	#0 pthread_create ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:969 (libtsan.so.0+0x605f8)
	# 	#1 std::sys::unix::thread::Thread::new::h4e3360b8718ad78c library/std/src/sys/unix/thread.rs:87 (inlines_test-93e5a5076af88165+0x7c47d)

	# TODO: For solving this maybe we can implement supression files, as maybe they are false positives
	return()
endif()

add_test(NAME ${target}
	COMMAND ${Rust_CARGO_EXECUTABLE} test ${RUN_INTEGRATION_TESTS_ONLY}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)
include(Portability)

project_library_path(TEST_LIB_PATH
	${PROJECT_OUTPUT_DIR}
)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	"${PROJECT_LIBRARY_PATH_NAME}=${TEST_LIB_PATH}"
	"CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
	"PROJECT_OUTPUT_DIR=${PROJECT_OUTPUT_DIR}"
	"RUST_BACKTRACE=1"
	${SANITIZER_FLAGS}
)
