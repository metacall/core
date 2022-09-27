# Check if this loader is enabled
if(NOT (OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_WASM AND OPTION_BUILD_LOADERS_PY
    AND OPTION_BUILD_SCRIPTS AND OPTION_BUILD_SCRIPTS_WASM AND OPTION_BUILD_SCRIPTS_PY
    AND OPTION_BUILD_PORTS AND OPTION_BUILD_PORTS_PY))
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-wasm-python-port-test)
message(STATUS "Test ${target}")

#
# Compiler warnings
#

include(Warnings)

#
# Compiler security
#

include(SecurityFlags)

#
# Sources
#

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(sources
	${source_path}/main.cpp
	${source_path}/metacall_wasm_python_port_test.cpp
)

# Group source files
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$"
	${header_group} ${headers})
source_group_by_path(${source_path}  "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$"
	${source_group} ${sources})

#
# Create executable
#

# Build executable
add_executable(${target}
	${sources}
)

# Create namespaced alias
add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

#
# Project options
#

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

#
# Include directories
#

target_include_directories(${target}
	PRIVATE
	${DEFAULT_INCLUDE_DIRECTORIES}
	${PROJECT_BINARY_DIR}/source/include
)

#
# Libraries
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LIBRARIES}

	GTest

	${META_PROJECT_NAME}::metacall
)

#
# Compile definitions
#

target_compile_definitions(${target}
	PRIVATE
	${DEFAULT_COMPILE_DEFINITIONS}
)

#
# Compile options
#

target_compile_options(${target}
	PRIVATE
	${DEFAULT_COMPILE_OPTIONS}
)

#
# Linker options
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Define test
#

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
)

#
# Define dependencies
#

add_dependencies(${target}
	wasm_loader
	py_loader
)

#
# Define test properties
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)


if(OPTION_BUILD_THREAD_SANITIZER)
	# TODO: This test fails when run with sanitizers:
	#
	# WARNING: ThreadSanitizer: signal-unsafe call inside of a signal (pid=14007)
	# 	#0 malloc ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:647 (libtsan.so.2+0x3ebb8)
	# 	#1 alloc::alloc::alloc::h8eea693b11397046 /rustc/9bc8c42bb2f19e745a63f3445f1ac248fb015e53/library/alloc/src/alloc.rs:86 (libwasmtime.so+0x823ef8)
	# 	#2 alloc::alloc::Global::alloc_impl::hdd19e4608fbda4e9 /rustc/9bc8c42bb2f19e745a63f3445f1ac248fb015e53/library/alloc/src/alloc.rs:166 (libwasmtime.so+0x823ef8)
	# 	#3 _$LT$alloc..alloc..Global$u20$as$u20$core..alloc..Allocator$GT$::allocate::hf61e18fb7f49870e /rustc/9bc8c42bb2f19e745a63f3445f1ac248fb015e53/library/alloc/src/alloc.rs:226 (libwasmtime.so+0x823ef8)
	# 	#4 alloc::alloc::exchange_malloc::h1859a431711d6c17 /rustc/9bc8c42bb2f19e745a63f3445f1ac248fb015e53/library/alloc/src/alloc.rs:316 (libwasmtime.so+0x823ef8)
	# 	#5 alloc::boxed::Box$LT$T$GT$::new::h4ef787c2f87c345b /rustc/9bc8c42bb2f19e745a63f3445f1ac248fb015e53/library/alloc/src/boxed.rs:185 (libwasmtime.so+0x823ef8)
	# 	#6 _$LT$alloc..boxed..Box$LT$T$GT$$u20$as$u20$core..convert..From$LT$T$GT$$GT$::from::h26993fc6ee782f3a /rustc/9bc8c42bb2f19e745a63f3445f1ac248fb015e53/library/alloc/src/boxed.rs:1200 (libwasmtime.so+0x823ef8)
	# 	#7 std::sys_common::mutex::MovableMutex::new::hf02abe706afa4cd7 library/std/src/sys_common/mutex.rs:64 (libwasmtime.so+0x823ef8)
	# 	#8 function_wasm_interface_invoke /usr/local/metacall/source/loaders/wasm_loader/source/wasm_loader_function.c:172 (libwasm_loaderd.so+0x47db)
	# 	#9 function_call /usr/local/metacall/source/reflect/source/reflect_function.c:607 (libmetacalld.so+0x241e3)
	# 	#10 py_loader_impl_function_type_invoke /usr/local/metacall/source/loaders/py_loader/source/py_loader_impl.c:2003 (libpy_loaderd.so+0xe01f)
	# 	#11 <null> <null> (libpython3.9.so.1.0+0x10cc73)
	# 	#12 py_loader_impl_load_from_file_relative /usr/local/metacall/source/loaders/py_loader/source/py_loader_impl.c:3102 (libpy_loaderd.so+0xb5ae)
	# 	#13 py_loader_impl_load_from_file /usr/local/metacall/source/loaders/py_loader/source/py_loader_impl.c:3193 (libpy_loaderd.so+0xb7db)
	# 	#14 loader_impl_load_from_file /usr/local/metacall/source/loader/source/loader_impl.c:838 (libmetacalld.so+0x30944)
	# 	#15 loader_load_from_file /usr/local/metacall/source/loader/source/loader.c:307 (libmetacalld.so+0x2e0d1)
	# 	#16 metacall_load_from_file /usr/local/metacall/source/metacall/source/metacall.c:348 (libmetacalld.so+0x32bbf)
	# 	#17 metacall_wasm_python_port_test_CallWasmFromPython_Test::TestBody() /usr/local/metacall/source/tests/metacall_wasm_python_port_test/source/metacall_wasm_python_port_test.cpp:45 (metacall-wasm-python-port-testd+0x209c4)
	# 	#18 void testing::internal::HandleExceptionsInMethodIfSupported<testing::Test, void>(testing::Test*, void (testing::Test::*)(), char const*) <null> (metacall-wasm-python-port-testd+0x4df46)
	# 	#19 <null> <null> (libc.so.6+0x29209)
	#
	# SUMMARY: ThreadSanitizer: signal-unsafe call inside of a signal /rustc/9bc8c42bb2f19e745a63f3445f1ac248fb015e53/library/alloc/src/alloc.rs:86 in alloc::alloc::alloc::h8eea693b11397046
	#
	#
	# WARNING: ThreadSanitizer: signal-unsafe call inside of a signal (pid=14007)
	# 	#0 malloc ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:647 (libtsan.so.2+0x3ebb8)
	# 	#1 _ZN3std4sync4once4Once9call_once28_$u7b$$u7b$closure$u7d$$u7d$17h00ca29eb50828eb3E.llvm.2373258818034603441 <null> (libwasmtime.so+0x57da03)
	# 	#2 function_wasm_interface_invoke /usr/local/metacall/source/loaders/wasm_loader/source/wasm_loader_function.c:172 (libwasm_loaderd.so+0x47db)
	# 	#3 function_call /usr/local/metacall/source/reflect/source/reflect_function.c:607 (libmetacalld.so+0x241e3)
	# 	#4 py_loader_impl_function_type_invoke /usr/local/metacall/source/loaders/py_loader/source/py_loader_impl.c:2003 (libpy_loaderd.so+0xe01f)
	# 	#5 <null> <null> (libpython3.9.so.1.0+0x10cc73)
	# 	#6 py_loader_impl_load_from_file_relative /usr/local/metacall/source/loaders/py_loader/source/py_loader_impl.c:3102 (libpy_loaderd.so+0xb5ae)
	# 	#7 py_loader_impl_load_from_file /usr/local/metacall/source/loaders/py_loader/source/py_loader_impl.c:3193 (libpy_loaderd.so+0xb7db)
	# 	#8 loader_impl_load_from_file /usr/local/metacall/source/loader/source/loader_impl.c:838 (libmetacalld.so+0x30944)
	# 	#9 loader_load_from_file /usr/local/metacall/source/loader/source/loader.c:307 (libmetacalld.so+0x2e0d1)
	# 	#10 metacall_load_from_file /usr/local/metacall/source/metacall/source/metacall.c:348 (libmetacalld.so+0x32bbf)
	# 	#11 metacall_wasm_python_port_test_CallWasmFromPython_Test::TestBody() /usr/local/metacall/source/tests/metacall_wasm_python_port_test/source/metacall_wasm_python_port_test.cpp:45 (metacall-wasm-python-port-testd+0x209c4)
	# 	#12 void testing::internal::HandleExceptionsInMethodIfSupported<testing::Test, void>(testing::Test*, void (testing::Test::*)(), char const*) <null> (metacall-wasm-python-port-testd+0x4df46)
	# 	#13 <null> <null> (libc.so.6+0x29209)
	#
	# SUMMARY: ThreadSanitizer: signal-unsafe call inside of a signal (/usr/local/metacall/build/wasmtime/wasmtime-v0.28.0-x86_64-linux-c-api/lib/libwasmtime.so+0x57da03) in _ZN3std4sync4once4Once9call_once28_$u7b$$u7b$closure$u7d$$u7d$17h00ca29eb50828eb3E.llvm.2373258818034603441
	#
	# ...
	#
	#
	# For solving this, we should enable WASM support for sanitizers and debug it properly
	set_tests_properties(${target} PROPERTIES
		PASS_REGULAR_EXPRESSION "[  PASSED  ]"
	)
endif()

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)
