#
# Unix dynamic loader library finder by Parra Studios
# Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Defines unix dynamic load library and include paths
#

if(LIBDL_INCLUDE_DIR)
    set(LIBDL_FIND_QUIETLY TRUE)
endif()

find_path(LIBDL_INCLUDE_DIR dlfcn.h)

set(LIBDL_NAMES dl libdl ltdl libltdl)

find_library(LIBDL_LIBRARY NAMES ${LIBDL_NAMES})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LibDL DEFAULT_MSG LIBDL_LIBRARY LIBDL_INCLUDE_DIR)

if(LIBDL_FOUND)
    set(LIBDL_LIBRARIES ${LIBDL_LIBRARY})
else()
    set(LIBDL_LIBRARIES)
endif()

mark_as_advanced(LIBDL_LIBRARY LIBDL_INCLUDE_DIR)
