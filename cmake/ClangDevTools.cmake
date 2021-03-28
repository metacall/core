# See FindClangFormat.cmake
# Variables of interest on this file: ${CLANG_FORMAT_VERSION} and ${CLANG_FORMAT_EXECUTABLE}

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
	COMMAND ${CLANG_FORMAT_EXECUTABLE}
	--verbose
	-style=file
	-i
	${ALL_SOURCE_FILES}
)
