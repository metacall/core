#
# The module defines the following variables
#
# ``ClangTidy_EXECUTABLE`` Path to clang-tidy executable
# ``ClangTidy_FOUND`` True if the clang-tidy executable was found.
# ``ClangTidy_VERSION`` The version of clang-tidy found
# ``ClangTidy_VERSION_MAJOR`` The clang-tidy major version if specified, 0
# otherwise ``ClangTidy_VERSION_MINOR`` The clang-tidy minor version if
# specified, 0 otherwise ``ClangTidy_VERSION_PATCH`` The clang-tidy patch
# version if specified, 0 otherwise ``ClangTidy_VERSION_COUNT`` Number of
# version components reported by clang-tidy
#
# Example usage:
#
# .. code-block:: cmake
#
# find_package(ClangTidy) if(ClangTidy_FOUND) message("clang-tidy
# executable found: ${ClangTidy_EXECUTABLE}\n" "version:
# ${ClangTidy_VERSION}") endif()

if(ClangTidy_FOUND)
	set(ClangTidy_FIND_QUIETLY TRUE)
endif()

set(ClangTidy_NAMES
	clang-tidy
	clang-tidy-11
	clang-tidy-12
)

set(ClangTidy_PATHS
	/usr/bin
	/usr/lib/llvm-11/bin
	/usr/lib/llvm-12/bin
)

find_program(ClangTidy_EXECUTABLE
	NAMES ${ClangTidy_NAMES}
	DOC "clang-tidy executable"
	PATHS ${ClangTidy_PATHS}
)

find_program(
	ClangTidy_RUN_EXECUTABLE
	NAMES
		run-clang-tidy
		run-clang-tidy.py
)

# Extract version from command "clang-tidy --version"
if(ClangTidy_EXECUTABLE)
	execute_process(COMMAND ${ClangTidy_EXECUTABLE} --version
					OUTPUT_VARIABLE clang_tidy_version
					ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

	if(clang_tidy_version MATCHES "LLVM version")
		# clang_tidy_version sample: "LLVM (http://llvm.org/):\n LLVM version 22.1.8\nOptimized build.)"
		string(REGEX
					REPLACE ".*LLVM version ([.0-9]+).*"
					"\\1"
					ClangTidy_VERSION
					"${clang_tidy_version}")
		# ClangTidy_VERSION sample: "22.1.8"

		# Extract version components
		string(REPLACE "." ";" clang_tidy_version "${ClangTidy_VERSION}")
		list(LENGTH clang_tidy_version ClangTidy_VERSION_COUNT)
		if(ClangTidy_VERSION_COUNT GREATER 0)
			list(GET clang_tidy_version 0 ClangTidy_VERSION_MAJOR)
		else()
			set(ClangTidy_VERSION_MAJOR 0)
		endif()
		if(ClangTidy_VERSION_COUNT GREATER 1)
			list(GET clang_tidy_version 1 ClangTidy_VERSION_MINOR)
		else()
			set(ClangTidy_VERSION_MINOR 0)
		endif()
		if(ClangTidy_VERSION_COUNT GREATER 2)
			list(GET clang_tidy_version 2 ClangTidy_VERSION_PATCH)
		else()
			set(ClangTidy_VERSION_PATCH 0)
		endif()
	endif()
	unset(clang_tidy_version)
endif()

if(ClangTidy_EXECUTABLE AND ClangTidy_VERSION)
	set(ClangTidy_FOUND TRUE)

	include(FindPackageHandleStandardArgs)

	# Set standard args
	find_package_handle_standard_args(ClangTidy
		REQUIRED_VARS ClangTidy_EXECUTABLE
		VERSION_VAR ClangTidy_VERSION
	)

	mark_as_advanced(ClangTidy_EXECUTABLE)
else()
	set(ClangTidy_FOUND FALSE)
endif()
