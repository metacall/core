#
#CMake Find Clang library by Parra Studios
#

if(LibClang_FOUND)
set(LibClang_FIND_QUITELY TRUE)
endif()

option(LibClang_CMAKE_DEBUG "Print paths for debugging LibClang dependencies." OFF)

if(LibClang_FIND_VERSION)
set(LibClang_VERSION_LIST ${LibClang_FIND_VERSION})
else()
set(LibClang_VERSION_LIST 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6.0 5.0 4.0 3.9 3.8)
endif()

macro(_libclang_generate_search_paths template result)
set(${result})
foreach(version ${LibClang_VERSION_LIST})
string(REPLACE "VERSION" "${version}" output_path "${template}")
list(APPEND ${result} ${output_path})
endforeach()
endmacro()

include(FindPackageHandleStandardArgs)

if(WIN32)
set(LibClang_WINDOWS_PATHS
"$ENV{LLVM_ROOT}"
"${LLVM_ROOT}"
"C:/Program Files/LLVM"
"C:/Program Files (x86)/LLVM"
"$ENV{VCPKG_ROOT}/installed/x64-windows"
"C:/vcpkg/installed/x64-windows"
"$ENV{USERPROFILE}/scoop/apps/llvm/current"
"C:/ProgramData/chocolatey/lib/llvm/tools/llvm"
)
if(NOT LibClang_LIBRARY)
find_library(LibClang_LIBRARY
NAMES libclang clang
PATHS ${LibClang_WINDOWS_PATHS}
PATH_SUFFIXES lib bin
)
endif()
set(LibClang_INCLUDE_HEADERS Index.h CXString.h CXErrorCode.h)
if(NOT LibClang_INCLUDE_DIR)
find_path(LibClang_INCLUDE_DIR
NAMES ${LibClang_INCLUDE_HEADERS}
PATHS ${LibClang_WINDOWS_PATHS}
PATH_SUFFIXES include/clang-c include
)
if(LibClang_INCLUDE_DIR)
get_filename_component(_parent "${LibClang_INCLUDE_DIR}" DIRECTORY)
if(EXISTS "${_parent}/clang-c/Index.h")
set(LibClang_INCLUDE_DIR "${_parent}")
endif()
endif()
endif()
find_file(LibClang_DLL
NAMES libclang.dll
PATHS ${LibClang_WINDOWS_PATHS}
PATH_SUFFIXES bin
)
else()
_libclang_generate_search_paths("/usr/lib/llvm-VERSION/lib/" LibClang_LIBRARY_PATHS)
if(NOT LibClang_LIBRARY)
find_library(LibClang_LIBRARY
NAMES clang
PATHS ${LibClang_LIBRARY_PATHS} /usr/lib
)
endif()
_libclang_generate_search_paths("/usr/lib/llvm-VERSION/include/clang-c" LibClang_INCLUDE_PATHS)
set(LibClang_INCLUDE_HEADERS
BuildSystem.h CXCompilationDatabase.h CXErrorCode.h
CXString.h Documentation.h Index.h Platform.h
)
if(NOT LibClang_INCLUDE_DIR)
find_path(LibClang_INCLUDE_DIR
NAMES ${LibClang_INCLUDE_HEADERS}
PATHS ${LibClang_INCLUDE_PATHS} /usr/include/clang-c
)
get_filename_component(LibClang_INCLUDE_DIR ${LibClang_INCLUDE_DIR} DIRECTORY)
endif()
endif()

find_package_handle_standard_args(LibClang DEFAULT_MSG LibClang_LIBRARY LibClang_INCLUDE_DIR)
mark_as_advanced(LibClang_LIBRARY LibClang_INCLUDE_DIR)

if(LibClang_CMAKE_DEBUG)
message(STATUS "LibClang_INCLUDE_DIR: ${LibClang_INCLUDE_DIR}")
message(STATUS "LibClang_LIBRARY: ${LibClang_LIBRARY}")
endif()
