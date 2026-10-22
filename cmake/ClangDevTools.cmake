# See FindClangFormat.cmake
# Variables of interest on this file: ${ClangFormat_VERSION} - ${ClangFormat_EXECUTABLE} and ${ClangTidy_VERSION} - ${ClangTidy_EXECUTABLE}

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

if(ClangTidy_FOUND AND ClangTidy_RUN_EXECUTABLE)
	find_package(
		Python3
		COMPONENTS Interpreter
		REQUIRED
	)

# Include only repository sources, using either platform's path separator.
string(REPLACE "/" [=[[/\\]]=] ClangTidy_SOURCE_REGEX "${CMAKE_SOURCE_DIR}/source/")
set(ClangTidy_SOURCE_REGEX "^${ClangTidy_SOURCE_REGEX}")

# These tests contain empty variadic macros that Clang cannot parse
# with the Windows compilation commands.
if(WIN32)
    string(APPEND ClangTidy_SOURCE_REGEX [=[(?!tests[/\\])]=])
endif()

add_custom_target(
    clang-tidy
    COMMAND
        ${Python3_EXECUTABLE}
        ${ClangTidy_RUN_EXECUTABLE}
        -p=${CMAKE_BINARY_DIR}
        -clang-tidy-binary=${ClangTidy_EXECUTABLE}
        -checks=-*,bugprone-*,clang-analyzer-*,modernize-use-nullptr,readability-braces-around-statements
        ${ClangTidy_SOURCE_REGEX}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM
)
endif()

if(ClangFormat_FOUND)
	add_custom_target(
		clang-format
		COMMAND ${ClangFormat_EXECUTABLE}
		--verbose
		-style=file
		-i
		${ALL_SOURCE_FILES}
	)
endif()
