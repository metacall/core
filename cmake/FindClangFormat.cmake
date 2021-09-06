#
# Taken from https://raw.githubusercontent.com/BlueBrain/git-cmake-format/master/FindClangFormat.cmake
# ---------------
#
# The module defines the following variables
#
# ``ClangFormat_EXECUTABLE`` Path to clang-format executable
# ``ClangFormat_FOUND`` True if the clang-format executable was found.
# ``ClangFormat_VERSION`` The version of clang-format found
# ``ClangFormat_VERSION_MAJOR`` The clang-format major version if specified, 0
# otherwise ``ClangFormat_VERSION_MINOR`` The clang-format minor version if
# specified, 0 otherwise ``ClangFormat_VERSION_PATCH`` The clang-format patch
# version if specified, 0 otherwise ``ClangFormat_VERSION_COUNT`` Number of
# version components reported by clang-format
#
# Example usage:
#
# .. code-block:: cmake
#
# find_package(ClangFormat) if(ClangFormat_FOUND) message("clang-format
# executable found: ${ClangFormat_EXECUTABLE}\n" "version:
# ${ClangFormat_VERSION}") endif()

if(ClangFormat_FOUND)
	set(ClangFormat_FIND_QUIETLY TRUE)
endif()

set(ClangFormat_NAMES
	clang-format
	clang-format-11
	clang-format-12
)

set(ClangFormat_PATHS
	/usr/bin
	/usr/lib/llvm-11/bin
	/usr/lib/llvm-12/bin
)

find_program(ClangFormat_EXECUTABLE
	NAMES ${ClangFormat_NAMES}
	DOC "clang-format executable"
	PATHS ${ClangFormat_PATHS}
)

# Extract version from command "clang-format -version"
if(ClangFormat_EXECUTABLE)
	execute_process(COMMAND ${ClangFormat_EXECUTABLE} -version
					OUTPUT_VARIABLE clang_format_version
					ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

	if(clang_format_version MATCHES "clang-format version .*")
		# clang_format_version sample: "clang-format version 3.9.1-4ubuntu3~16.04.1
		# (tags/RELEASE_391/rc2)"
		string(REGEX
					REPLACE ".*clang-format version ([.0-9]+).*"
					"\\1"
					ClangFormat_VERSION
					"${clang_format_version}")
		# ClangFormat_VERSION sample: "3.9.1"

		# Extract version components
		string(REPLACE "." ";" clang_format_version "${ClangFormat_VERSION}")
		list(LENGTH clang_format_version ClangFormat_VERSION_COUNT)
		if(ClangFormat_VERSION_COUNT GREATER 0)
			list(GET clang_format_version 0 ClangFormat_VERSION_MAJOR)
		else()
			set(ClangFormat_VERSION_MAJOR 0)
		endif()
		if(ClangFormat_VERSION_COUNT GREATER 1)
			list(GET clang_format_version 1 ClangFormat_VERSION_MINOR)
		else()
			set(ClangFormat_VERSION_MINOR 0)
		endif()
		if(ClangFormat_VERSION_COUNT GREATER 2)
			list(GET clang_format_version 2 ClangFormat_VERSION_PATCH)
		else()
			set(ClangFormat_VERSION_PATCH 0)
		endif()
	endif()
	unset(clang_format_version)
endif()

if(ClangFormat_EXECUTABLE AND ClangFormat_VERSION)
	set(ClangFormat_FOUND TRUE)

	include(FindPackageHandleStandardArgs)

	# Set standard args
	find_package_handle_standard_args(ClangFormat
		REQUIRED_VARS ClangFormat_EXECUTABLE
		VERSION_VAR ClangFormat_VERSION
	)

	mark_as_advanced(ClangFormat_EXECUTABLE)
else()
	set(ClangFormat_FOUND FALSE)
endif()
