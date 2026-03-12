# See FindClangFormat.cmake
# Variables of interest on this file: ${ClangFormat_VERSION}, ${ClangFormat_EXECUTABLE}, ${ClangFormat_USE_DOCKER}

# Get only C/C++ files for now
file(GLOB_RECURSE
	ALL_SOURCE_FILES
	LIST_DIRECTORIES OFF
	FOLLOW_SYMLINKS
	${CMAKE_SOURCE_DIR}/source/**/*.cpp
	${CMAKE_SOURCE_DIR}/source/**/*.hpp
	${CMAKE_SOURCE_DIR}/source/**/*.h
	${CMAKE_SOURCE_DIR}/source/**/*.c
	${CMAKE_SOURCE_DIR}/source/**/*.cc
	${CMAKE_SOURCE_DIR}/source/**/*.hh
	${CMAKE_SOURCE_DIR}/source/**/*.cxx
	${CMAKE_SOURCE_DIR}/source/**/*.inl
)

if(ClangFormat_USE_DOCKER)
	message(STATUS
		"clang-format target will run via Docker (ghcr.io/jidicula/clang-format:12). "
		"Pulling image now to avoid latency at build time..."
	)

	# Pull the image at configure time so `cmake --build . --target clang-format`
	# does not stall on the first run waiting for the pull.
	execute_process(
		COMMAND docker pull ghcr.io/jidicula/clang-format:12
		RESULT_VARIABLE _docker_pull_result
		OUTPUT_QUIET
		ERROR_QUIET
	)

	if(NOT _docker_pull_result EQUAL 0)
		message(WARNING
			"Failed to pull ghcr.io/jidicula/clang-format:12 at configure time. "
			"The image will be pulled on the first `clang-format` build target invocation, "
			"or the pull may fail if there is no network access / Docker daemon is not running."
		)
	else()
		message(STATUS "Docker image ghcr.io/jidicula/clang-format:12 is ready.")
	endif()

	unset(_docker_pull_result)
else()
	message(STATUS "clang-format target will use native binary: ${ClangFormat_EXECUTABLE} (version ${ClangFormat_VERSION})")
endif()

# clang-tidy not implemented yet
#add_custom_target(
#	clang-tidy
#	COMMAND /usr/bin/clang-tidy
#	${ALL_SOURCE_FILES}
#	-config=''
#	--
#	-std=c++11
#	${INCLUDE_DIRECTORIES}
#)

add_custom_target(
	clang-format
	COMMAND ${ClangFormat_EXECUTABLE}
	--verbose
	-style=file
	-i
	${ALL_SOURCE_FILES}
	COMMENT "Running clang-format 12 on all source files..."
)
