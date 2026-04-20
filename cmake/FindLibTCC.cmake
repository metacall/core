#
#CMake Find Tiny C Compiler library by Parra Studios
#

if(LIBTCC_FOUND)
set(LIBTCC_FIND_QUITELY TRUE)
endif()

include(FindPackageHandleStandardArgs)

set(LIBTCC_SUFFIXES
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

if(WIN32)
set(LIBTCC_WINDOWS_PATHS
"$ENV{TCC_ROOT}"
"${TCC_ROOT}"
"$ENV{USERPROFILE}/scoop/apps/tcc/current"
"C:/tcc"
"C:/Program Files/tcc"
"C:/ProgramData/tcc"
)
find_library(LIBTCC_LIBRARY
NAMES libtcc tcc
PATHS ${LIBTCC_WINDOWS_PATHS}
PATH_SUFFIXES lib . lib/tcc
)
find_path(LIBTCC_INCLUDE_DIR libtcc.h
PATHS ${LIBTCC_WINDOWS_PATHS}
PATH_SUFFIXES include .
)
find_file(LIBTCC_DLL
NAMES libtcc.dll tcc.dll
PATHS ${LIBTCC_WINDOWS_PATHS}
PATH_SUFFIXES bin .
)
else()
set(LIBTCC_LIBRARY_NAMES
${CMAKE_SHARED_LIBRARY_PREFIX}tcc${CMAKE_SHARED_LIBRARY_SUFFIX}
)
find_library(LIBTCC_LIBRARY
NAMES ${LIBTCC_LIBRARY_NAMES}
PATHS /usr /usr/lib /usr/local /opt/local
PATH_SUFFIXES lib lib64 ${LIBTCC_SUFFIXES}
)
find_path(LIBTCC_INCLUDE_DIR libtcc.h
PATHS /usr /usr/include /usr/local /opt/local
PATH_SUFFIXES ${LIBTCC_SUFFIXES}
)
endif()

find_package_handle_standard_args(LibTCC DEFAULT_MSG LIBTCC_LIBRARY LIBTCC_INCLUDE_DIR)
mark_as_advanced(LIBTCC_INCLUDE_DIR LIBTCC_LIBRARY)
