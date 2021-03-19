#
# Taken from https://raw.githubusercontent.com/BlueBrain/git-cmake-format/master/FindClangFormat.cmake
# ---------------
#
# The module defines the following variables
#
# ``CLANG_FORMAT_EXECUTABLE`` Path to clang-format executable
# ``CLANG_FORMAT_FOUND`` True if the clang-format executable was found.
# ``CLANG_FORMAT_VERSION`` The version of clang-format found
# ``CLANG_FORMAT_VERSION_MAJOR`` The clang-format major version if specified, 0
# otherwise ``CLANG_FORMAT_VERSION_MINOR`` The clang-format minor version if
# specified, 0 otherwise ``CLANG_FORMAT_VERSION_PATCH`` The clang-format patch
# version if specified, 0 otherwise ``CLANG_FORMAT_VERSION_COUNT`` Number of
# version components reported by clang-format
#
# Example usage:
#
# .. code-block:: cmake
#
# find_package(ClangFormat) if(CLANG_FORMAT_FOUND) message("clang-format
# executable found: ${CLANG_FORMAT_EXECUTABLE}\n" "version:
# ${CLANG_FORMAT_VERSION}") endif()

find_program(CLANG_FORMAT_EXECUTABLE
             NAMES clang-format-11
                   clang-format
                   clang-format-10
                   clang-format-12
                   clang-format-9
             DOC "clang-format executable")
mark_as_advanced(CLANG_FORMAT_EXECUTABLE)

# Extract version from command "clang-format -version"
if(CLANG_FORMAT_EXECUTABLE)
  execute_process(COMMAND ${CLANG_FORMAT_EXECUTABLE} -version
                  OUTPUT_VARIABLE clang_format_version
                  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(clang_format_version MATCHES "clang-format version .*")
    # clang_format_version sample: "clang-format version 3.9.1-4ubuntu3~16.04.1
    # (tags/RELEASE_391/rc2)"
    string(REGEX
           REPLACE ".*clang-format version ([.0-9]+).*"
                   "\\1"
                   CLANG_FORMAT_VERSION
                   "${clang_format_version}")
    # CLANG_FORMAT_VERSION sample: "3.9.1"

    # Extract version components
    string(REPLACE "."
                   ";"
                   clang_format_version
                   "${CLANG_FORMAT_VERSION}")
    list(LENGTH clang_format_version CLANG_FORMAT_VERSION_COUNT)
    if(CLANG_FORMAT_VERSION_COUNT GREATER 0)
      list(GET clang_format_version 0 CLANG_FORMAT_VERSION_MAJOR)
    else()
      set(CLANG_FORMAT_VERSION_MAJOR 0)
    endif()
    if(CLANG_FORMAT_VERSION_COUNT GREATER 1)
      list(GET clang_format_version 1 CLANG_FORMAT_VERSION_MINOR)
    else()
      set(CLANG_FORMAT_VERSION_MINOR 0)
    endif()
    if(CLANG_FORMAT_VERSION_COUNT GREATER 2)
      list(GET clang_format_version 2 CLANG_FORMAT_VERSION_PATCH)
    else()
      set(CLANG_FORMAT_VERSION_PATCH 0)
    endif()
  endif()
  unset(clang_format_version)
endif()

if(CLANG_FORMAT_EXECUTABLE)
  set(CLANG_FORMAT_FOUND TRUE)
else()
  set(CLANG_FORMAT_FOUND FALSE)
endif()
