#
# Find clang-format
#
# Usage:
#
#	find_package(ClangFormat [<major>] [EXACT] [REQUIRED] [QUIET])
#
# When a version is requested, the executable clang-format-<major> and the
# installation paths of that LLVM version are searched first. With EXACT,
# only that major version is accepted (minor and patch are not compared,
# because clang-format formats the same within a major version).
#
# Input variables:
#
#	ClangFormat_EXECUTABLE   Path to the executable, set it to skip the search
#
# Result variables:
#
#	ClangFormat_FOUND           True if a suitable clang-format was found
#	ClangFormat_EXECUTABLE      Path to the clang-format executable
#	ClangFormat_VERSION         Full version (i.e: 19.1.7)
#	ClangFormat_VERSION_MAJOR   Major version (i.e: 19)
#	ClangFormat_VERSION_MINOR   Minor version (i.e: 1)
#	ClangFormat_VERSION_PATCH   Patch version (i.e: 7)
#

# Search the executable of the requested major version first:
#	1) clang-format-<major> in PATH or in the LLVM installation paths
#	2) clang-format in the LLVM installation paths of that version
# and then fall back to the generic search
if(NOT ClangFormat_EXECUTABLE AND ClangFormat_FIND_VERSION_MAJOR)
	set(ClangFormat_MAJOR "${ClangFormat_FIND_VERSION_MAJOR}")

	set(ClangFormat_HINTS
		/usr/lib/llvm-${ClangFormat_MAJOR}/bin            # Debian, Ubuntu
		/usr/lib/llvm${ClangFormat_MAJOR}/bin             # Alpine
		/usr/local/llvm${ClangFormat_MAJOR}/bin           # FreeBSD
		/opt/homebrew/opt/llvm@${ClangFormat_MAJOR}/bin   # macOS Homebrew (Apple Silicon)
		/usr/local/opt/llvm@${ClangFormat_MAJOR}/bin      # macOS Homebrew (Intel)
	)

	find_program(ClangFormat_EXECUTABLE
		NAMES clang-format-${ClangFormat_MAJOR}
		HINTS ${ClangFormat_HINTS}
	)

	if(NOT ClangFormat_EXECUTABLE)
		find_program(ClangFormat_EXECUTABLE
			NAMES clang-format
			HINTS ${ClangFormat_HINTS}
			NO_DEFAULT_PATH
		)
	endif()

	unset(ClangFormat_HINTS)
	unset(ClangFormat_MAJOR)
endif()

# Generic search (PATH, and the default LLVM installation path on Windows)
find_program(ClangFormat_EXECUTABLE
	NAMES clang-format
	PATHS "$ENV{ProgramFiles}/LLVM/bin"
	DOC "clang-format executable"
)

mark_as_advanced(ClangFormat_EXECUTABLE)

# Obtain the version, the output has a vendor prefix depending on the
# distribution (i.e: "Ubuntu clang-format version 19.1.7 (6ubuntu3)")
unset(ClangFormat_VERSION)
unset(ClangFormat_VERSION_MAJOR)
unset(ClangFormat_VERSION_MINOR)
unset(ClangFormat_VERSION_PATCH)

if(ClangFormat_EXECUTABLE)
	execute_process(
		COMMAND "${ClangFormat_EXECUTABLE}" --version
		OUTPUT_VARIABLE ClangFormat_VERSION_OUTPUT
		RESULT_VARIABLE ClangFormat_VERSION_RESULT
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	if(ClangFormat_VERSION_RESULT EQUAL 0 AND ClangFormat_VERSION_OUTPUT MATCHES "clang-format version ([0-9]+)\\.([0-9]+)\\.([0-9]+)")
		set(ClangFormat_VERSION_MAJOR "${CMAKE_MATCH_1}")
		set(ClangFormat_VERSION_MINOR "${CMAKE_MATCH_2}")
		set(ClangFormat_VERSION_PATCH "${CMAKE_MATCH_3}")
		set(ClangFormat_VERSION "${ClangFormat_VERSION_MAJOR}.${ClangFormat_VERSION_MINOR}.${ClangFormat_VERSION_PATCH}")
	endif()

	unset(ClangFormat_VERSION_OUTPUT)
	unset(ClangFormat_VERSION_RESULT)
endif()

# With EXACT only the major version is compared, the standard check would
# compare the full version (19 against 19.1.7) and always fail
set(ClangFormat_VERSION_CHECK ClangFormat_VERSION)

if(ClangFormat_FIND_VERSION_EXACT AND ClangFormat_FIND_VERSION_COUNT EQUAL 1)
	set(ClangFormat_VERSION_CHECK ClangFormat_VERSION_MAJOR)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(ClangFormat
	REQUIRED_VARS ClangFormat_EXECUTABLE ClangFormat_VERSION
	VERSION_VAR ${ClangFormat_VERSION_CHECK}
)

unset(ClangFormat_VERSION_CHECK)

# Do not keep a cached executable with the wrong version, so the next
# configuration searches again (i.e: after installing the right version)
if(NOT ClangFormat_FOUND AND ClangFormat_EXECUTABLE AND ClangFormat_FIND_VERSION_EXACT)
	set(ClangFormat_EXECUTABLE_REJECTED "${ClangFormat_EXECUTABLE}")
	unset(ClangFormat_EXECUTABLE CACHE)

	# Keep it as a normal variable, so the caller can report what was found
	set(ClangFormat_EXECUTABLE "${ClangFormat_EXECUTABLE_REJECTED}")
	unset(ClangFormat_EXECUTABLE_REJECTED)
endif()
