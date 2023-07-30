#
#	CMake Install Tiny C Compiler by Parra Studios
#	CMake script to install TCC library.
#
#	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
#

# The following variables are set:
#
# LIBTCC_FOUND - True if TCC library was found
# LIBTCC_TARGET - Defines the TCC depends target (for using it as dependency on other targets)
# LIBTCC_INCLUDE_DIR - TCC headers path
# LIBTCC_LIBRARY - List of TCC libraries

if(LIBTCC_FOUND)
	return()
endif()

include(Portability)

set(LIBTCC_INSTALL_PREFIX "${PROJECT_OUTPUT_DIR}/libtcc")
file(MAKE_DIRECTORY ${LIBTCC_INSTALL_PREFIX})

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	set(LIBTCC_DEBUG "--debug")
else()
	set(LIBTCC_DEBUG)
endif()

# Configure
if(PROJECT_OS_FAMILY STREQUAL unix)
	if(OPTION_BUILD_MUSL)
		set(LIBTCC_CONFIGURE ./configure --prefix=${LIBTCC_INSTALL_PREFIX} ${LIBTCC_DEBUG} --disable-static --config-musl)
	else()
		set(LIBTCC_CONFIGURE ./configure --prefix=${LIBTCC_INSTALL_PREFIX} ${LIBTCC_DEBUG} --disable-static --with-libgcc --with-selinux)
	endif()
elseif(PROJECT_OS_FAMILY STREQUAL macos)
	# TODO: --disable-static is not working on MacOS, this should be reported or further investigated

	# AddressSanitizer:DEADLYSIGNAL
	# =================================================================
	# ==5339==ERROR: AddressSanitizer: BUS on unknown address 0x7fffac377b10 (pc 0x7fffac377b10 bp 0x7ffee8c2a0a0 sp 0x7ffee8c29f98 T0)
	#     #0 0x7fffac377b0f in off32 (libsystem_c.dylib:x86_64+0x3647db0f)
	#     #1 0x10bab502d in parse_btype tccgen.c
	#     #2 0x10babcd02 in decl0 tccgen.c:8197
	#     #3 0x10baa74d8 in tccgen_compile tccgen.c:8449
	#     #4 0x10ba90c48 in tcc_compile libtcc.c:742
	#     #5 0x10ba9186a in tcc_add_file_internal libtcc.c:1101
	#     #6 0x10b9e8db1 in c_loader_impl_load_from_file c_loader_impl.cpp:933
	#     #7 0x1071b8fc5 in loader_impl_load_from_file loader_impl.c:842
	#     #8 0x1071b302d in loader_load_from_file loader.c:317
	#     #9 0x1071c9488 in metacall_load_from_file metacall.c:348
	#     #10 0x106fd7ce1 in metacall_c_test_DefaultConstructor_Test::TestBody() metacall_c_test.cpp:53
	#     #11 0x10704125d in void testing::internal::HandleSehExceptionsInMethodIfSupported<testing::Test, void>(testing::Test*, void (testing::Test::*)(), char const*) (metacall-c-testd:x86_64+0x10006d25d)
	#     #12 0x107002e9a in void testing::internal::HandleExceptionsInMethodIfSupported<testing::Test, void>(testing::Test*, void (testing::Test::*)(), char const*) (metacall-c-testd:x86_64+0x10002ee9a)
	#     #13 0x107002dd2 in testing::Test::Run() (metacall-c-testd:x86_64+0x10002edd2)
	#     #14 0x107004011 in testing::TestInfo::Run() (metacall-c-testd:x86_64+0x100030011)
	#     #15 0x1070051ff in testing::TestSuite::Run() (metacall-c-testd:x86_64+0x1000311ff)
	#     #16 0x107014ce5 in testing::internal::UnitTestImpl::RunAllTests() (metacall-c-testd:x86_64+0x100040ce5)
	#     #17 0x10704570d in bool testing::internal::HandleSehExceptionsInMethodIfSupported<testing::internal::UnitTestImpl, bool>(testing::internal::UnitTestImpl*, bool (testing::internal::UnitTestImpl::*)(), char const*) (metacall-c-testd:x86_64+0x10007170d)
	#     #18 0x10701465a in bool testing::internal::HandleExceptionsInMethodIfSupported<testing::internal::UnitTestImpl, bool>(testing::internal::UnitTestImpl*, bool (testing::internal::UnitTestImpl::*)(), char const*) (metacall-c-testd:x86_64+0x10004065a)
	#     #19 0x10701452f in testing::UnitTest::Run() (metacall-c-testd:x86_64+0x10004052f)
	#     #20 0x106fd6049 in RUN_ALL_TESTS() gtest.h:2490
	#     #21 0x106fd5ee3 in main main.cpp:27
	#     #22 0x7fff75eb03d4 in start (libdyld.dylib:x86_64+0x163d4)

	# ==5339==Register values:
	# rax = 0x000000000000002c  rbx = 0x0000000000000001  rcx = 0x00006250000062a0  rdx = 0x0000000000000000
	# rdi = 0x000000000000002c  rsi = 0x00007ffee8c2a028  rbp = 0x00007ffee8c2a0a0  rsp = 0x00007ffee8c29f98
	#  r8 = 0x000000010bb0f350   r9 = 0x0000000111c5457c  r10 = 0x0000000000000000  r11 = 0x00007fffac377b10
	# r12 = 0x0000000000000000  r13 = 0x00007ffee8c29fe0  r14 = 0x000000010bb13c50  r15 = 0x000000000000053b
	# AddressSanitizer can not provide additional info.
	# SUMMARY: AddressSanitizer: BUS (libsystem_c.dylib:x86_64+0x3647db0f) in off32

	set(LIBTCC_CONFIGURE ./configure --prefix=${LIBTCC_INSTALL_PREFIX} ${LIBTCC_DEBUG} --enable-cross) # --disable-static
elseif(PROJECT_OS_FAMILY STREQUAL win32)
	if(PROJECT_OS_NAME STREQUAL MinGW)
		set(LIBTCC_CONFIGURE ./configure --prefix=${LIBTCC_INSTALL_PREFIX} ${LIBTCC_DEBUG} --config-mingw32 --disable-static --with-libgcc --with-selinux)
	else()
		set(LIBTCC_CONFIGURE "")
	endif()
else()
	message(FATAL_ERROR "TCC library install support not implemented in this platform")
endif()

include(ProcessorCount)
ProcessorCount(N)

# Build
if(PROJECT_OS_BSD)
	set(LIBTCC_BUILD gmake -j${N})
elseif(PROJECT_OS_FAMILY STREQUAL unix)
	set(LIBTCC_BUILD make -j${N})
elseif(PROJECT_OS_FAMILY STREQUAL macos)
	set(LIBTCC_BUILD make -j${N} MACOSX_DEPLOYMENT_TARGET=${PROJECT_OS_VERSION})
elseif(PROJECT_OS_FAMILY STREQUAL win32)
	if(PROJECT_OS_NAME STREQUAL MinGW)
		set(LIBTCC_BUILD make -j${N})
	else()
		set(LIBTCC_BUILD ./win32/build-tcc.bat -i ${LIBTCC_INSTALL_PREFIX})
	endif()
else()
	message(FATAL_ERROR "TCC library install support not implemented in this platform")
endif()

# Install
if(PROJECT_OS_BSD)
	set(LIBTCC_INSTALL gmake install)
elseif(PROJECT_OS_FAMILY STREQUAL win32 AND PROJECT_OS_NAME STREQUAL Windows)
	set(LIBTCC_INSTALL "")
else()
	set(LIBTCC_INSTALL make install)
endif()

set(LIBTCC_TARGET libtcc-depends)
set(LIBTCC_COMMIT_SHA "afc1362")
if(PROJECT_OS_FAMILY STREQUAL macos)
	# TODO: --disable-static is not working on MacOS, this should be reported or further investigated, remove this when it is solved
	set(LIBTTC_LIBRARY_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}tcc${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
	set(LIBTTC_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}tcc${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()
set(LIBTTC_LIBRARY_PATH "${PROJECT_OUTPUT_DIR}/${LIBTTC_LIBRARY_NAME}")
set(LIBTTC_RUNTIME_PATH "${LIBTCC_INSTALL_PREFIX}/lib/tcc")
set(LIBTTC_RUNTIME_INCLUDE_PATH "${LIBTTC_RUNTIME_PATH}/include")
set(LIBTTC_RUNTIME_FILES
	"${LIBTTC_RUNTIME_PATH}/libtcc1.a"
	"${LIBTTC_RUNTIME_PATH}/bcheck.o"
	"${LIBTTC_RUNTIME_PATH}/bt-exe.o"
	"${LIBTTC_RUNTIME_PATH}/bt-log.o"
)

# LibTCC Proejct
ExternalProject_Add(${LIBTCC_TARGET}
	DOWNLOAD_NAME		tinycc.tar.gz
	URL					https://github.com/metacall/tinycc/archive/${LIBTCC_COMMIT_SHA}.tar.gz
	URL_MD5				5582b17ee5848aeec28bee13773843f7
	CONFIGURE_COMMAND	${LIBTCC_CONFIGURE}
	BUILD_COMMAND		${LIBTCC_BUILD}
	BUILD_IN_SOURCE		true
	TEST_COMMAND		""
	UPDATE_COMMAND		""
	INSTALL_COMMAND		${LIBTCC_INSTALL}
	COMMAND				${CMAKE_COMMAND} -E copy "${LIBTCC_INSTALL_PREFIX}/lib/${LIBTTC_LIBRARY_NAME}" "${LIBTTC_LIBRARY_PATH}"
	COMMAND				${CMAKE_COMMAND} -E copy ${LIBTTC_RUNTIME_FILES} "${PROJECT_OUTPUT_DIR}"
)

# Install Library
install(FILES
	${LIBTTC_LIBRARY_PATH}
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)

# Runtime files
install(DIRECTORY
	${LIBTTC_RUNTIME_PATH}/
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
	FILES_MATCHING
	PATTERN "*.a"
	PATTERN "*.o"
	PATTERN "include" EXCLUDE
)

# Header files
install(DIRECTORY
	${LIBTTC_RUNTIME_INCLUDE_PATH}/
	DESTINATION ${INSTALL_INCLUDE}
	COMPONENT runtime
)

set(LIBTCC_INCLUDE_DIR	"${LIBTCC_INSTALL_PREFIX}/include")
set(LIBTCC_LIBRARY		"${LIBTTC_LIBRARY_PATH}")
set(LIBTCC_FOUND		TRUE)

mark_as_advanced(LIBTCC_INCLUDE_DIR LIBTCC_LIBRARY)

message(STATUS "Installing LibTCC ${LIBTCC_COMMIT_SHA}")
