if(OPTION_BUILD_CACHE)
	find_package(SCCache REQUIRED)

	# The launcher variable also reads the environment variable of the same name, so an explicit choice wins
	foreach(CACHE_LANG C CXX)
		if(NOT CMAKE_${CACHE_LANG}_COMPILER_LAUNCHER)
			set(CMAKE_${CACHE_LANG}_COMPILER_LAUNCHER "${SCCACHE_EXECUTABLE}")
		endif()
	endforeach()

	# sccache cannot reuse /Zi objects, Embedded is /Z7 and needs CMake 3.25 with CMP0141
	if(MSVC AND NOT CMAKE_VERSION VERSION_LESS 3.25)
		cmake_policy(SET CMP0141 NEW)
		set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")
	endif()

	# The identity is derived from the options that decide the compiler flags, so it cannot drift from them
	if(NOT METACALL_CACHE_PROFILE)
		if(OPTION_BUILD_ADDRESS_SANITIZER)
			set(METACALL_CACHE_PROFILE "asan")
		elseif(OPTION_BUILD_THREAD_SANITIZER)
			set(METACALL_CACHE_PROFILE "tsan")
		elseif(OPTION_BUILD_MEMORY_SANITIZER)
			set(METACALL_CACHE_PROFILE "msan")
		elseif(OPTION_TEST_MEMORYCHECK)
			set(METACALL_CACHE_PROFILE "memcheck")
		elseif(OPTION_COVERAGE)
			set(METACALL_CACHE_PROFILE "coverage")
		else()
			set(METACALL_CACHE_PROFILE "${CMAKE_BUILD_TYPE}")
		endif()
	endif()

	if(NOT SCCACHE_VERSION)
		set(SCCACHE_VERSION "unknown")
	endif()

	string(TOLOWER "${METACALL_CACHE_PROFILE}" METACALL_CACHE_PROFILE)
	string(TOLOWER "${CMAKE_BUILD_TYPE}" METACALL_CACHE_BUILD_TYPE)

	if(NOT METACALL_CACHE_DIR)
		if(DEFINED ENV{METACALL_CACHE_DIR})
			set(METACALL_CACHE_DIR "$ENV{METACALL_CACHE_DIR}")
		elseif(DEFINED ENV{SCCACHE_DIR})
			set(METACALL_CACHE_DIR "$ENV{SCCACHE_DIR}")
		else()
			set(METACALL_CACHE_DIR "$ENV{HOME}/.cache/metacall/sccache")
		endif()
	endif()

	string(TOLOWER "${CMAKE_C_COMPILER_ID}-${CMAKE_C_COMPILER_VERSION}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}" METACALL_CACHE_TOOLCHAIN)
	set(METACALL_CACHE_ID "sccache-${METACALL_CACHE_TOOLCHAIN}-${METACALL_CACHE_BUILD_TYPE}")

	if(NOT METACALL_CACHE_PROFILE STREQUAL METACALL_CACHE_BUILD_TYPE)
		set(METACALL_CACHE_ID "${METACALL_CACHE_ID}-${METACALL_CACHE_PROFILE}")
	endif()

	# The CI cannot read the build directory inside an image layer, and the toolchain id keeps
	# the METACALL_CACHE_ID name free for tools/metacall-cache.sh
	file(WRITE "${CMAKE_BINARY_DIR}/metacall-compiler-cache.env"
		"METACALL_CACHE_PROFILE=${METACALL_CACHE_PROFILE}\n"
		"METACALL_CACHE_TOOLCHAIN_ID=${METACALL_CACHE_ID}\n"
		"METACALL_CACHE_DIR=${METACALL_CACHE_DIR}\n"
		"METACALL_CACHE_SCCACHE_VERSION=${SCCACHE_VERSION}\n"
		"METACALL_CACHE_LAUNCHER=${SCCACHE_EXECUTABLE}\n"
	)

	message(STATUS "Compiler cache: sccache ${SCCACHE_VERSION} ${METACALL_CACHE_ID}")
endif()
