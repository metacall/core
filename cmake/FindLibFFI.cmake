#
#CMake Find Foreing Function Interface library by Parra Studios
#

if(LIBFFI_INCLUDE_DIRS)
set(LIBFFI_FIND_QUITELY TRUE)
endif()

include(FindPackageHandleStandardArgs)

set(LIBFFI_SUFFIXES
ffi
x86_64-linux-gnu
aarch64-linux-gnu
arm-linux-gnueabi
arm-linux-gnueabihf
i386-linux-gnu
mips64el-linux-gnuabi64
mipsel-linux-gnu
powerpc64le-linux-gnu
s390x-linux-gnu
)

find_library(LIBFFI_LIBRARY
NAMES ffi libffi
PATHS /usr /usr/lib /usr/local /opt/local
PATH_SUFFIXES lib lib64 ${LIBFFI_SUFFIXES}
)

if(APPLE)
execute_process(COMMAND brew --prefix libffi OUTPUT_VARIABLE LIBFFI_PREFIXES)
elseif(WIN32)
set(LIBFFI_WINDOWS_PATHS
"$ENV{LIBFFI_ROOT}"
"${LIBFFI_ROOT}"
"$ENV{VCPKG_ROOT}/installed/x64-windows"
"$ENV{VCPKG_ROOT}/installed/x86-windows"
"C:/vcpkg/installed/x64-windows"
"C:/ProgramData/chocolatey/lib/libffi"
"$ENV{USERPROFILE}/scoop/apps/libffi/current"
"C:/libffi"
)
find_library(LIBFFI_LIBRARY
NAMES ffi libffi
PATHS ${LIBFFI_WINDOWS_PATHS}
PATH_SUFFIXES lib lib64 x64/lib x86/lib
)
find_path(LIBFFI_INCLUDE_DIR ffi.h
PATHS ${LIBFFI_WINDOWS_PATHS}
PATH_SUFFIXES include
)
else()
set(LIBFFI_PREFIXES)
endif()

if(NOT LIBFFI_INCLUDE_DIR)
find_path(LIBFFI_INCLUDE_DIR ffi.h
PATHS /usr /usr/include /usr/local /opt/local /usr/include/ffi
PATH_SUFFIXES ${LIBFFI_SUFFIXES}
HINT LIBFFI_PREFIXES
)
endif()

if(NOT WIN32)
if(NOT LIBFFI_LIBRARY OR NOT LIBFFI_INCLUDE_DIR)
find_package(PkgConfig)
pkg_check_modules(PC_LIBFFI QUIET libffi)
find_path(LIBFFI_INCLUDE_DIR ffi.h HINTS ${PC_LIBFFI_INCLUDEDIR} ${PC_LIBFFI_INCLUDE_DIRS})
find_library(LIBFFI_LIBRARY NAMES ffi HINTS ${PC_LIBFFI_LIBDIR} ${PC_LIBFFI_LIBRARY_DIRS})
endif()
endif()

find_package_handle_standard_args(LibFFI DEFAULT_MSG LIBFFI_LIBRARY LIBFFI_INCLUDE_DIR)
mark_as_advanced(LIBFFI_INCLUDE_DIR LIBFFI_LIBRARY)
