#
# CMake Find MetaCall library by Parra Studios
# Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Find MetaCall library and include paths
#
# METACALL_FOUND - True if MetaCall library was found
# METACALL_INCLUDE_DIRS - MetaCall headers path
# METACALL_LIBRARIES - List of MetaCall libraries
# METACALL_DEFINITIONS - MetaCall definitions

# Prevent vervosity if already included
if(METACALL_FOUND)
    set(METACALL_FIND_QUITELY TRUE)
endif()

# Optional root directory
set(METACALL_ROOT_DIR "$ENV{WORKSPACE_ROOT}/metacall")

# MetaCall Version include path
#find_path(METACALL_VERSION_INCLUDE_DIR
#	NAMES version/version.h
#	PATHS ${METACALL_ROOT_DIR}
#	DOC "MetaCall Version include directory"
#)

# MetaCall Version library path
find_library(METACALL_VERSION_LIBRARY
	NAMES version
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Version library"
)

# MetaCall Preprocessor include path
find_path(METACALL_PREPROCESSOR_INCLUDE_DIR
	NAMES preprocessor/preprocessor.h
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Preprocessor include directory"
)

# MetaCall Preprocessor library path
find_library(METACALL_PREPROCESSOR_LIBRARY
	NAMES preprocessor
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Preprocessor library"
)

# MetaCall Format include path
find_path(METACALL_FORMAT_INCLUDE_DIR
	NAMES format/format.h
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Format include directory"
)

# MetaCall Format library path
find_library(METACALL_FORMAT_LIBRARY
	NAMES format
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Format library"
)

# MetaCall Log include path
find_path(METACALL_LOG_INCLUDE_DIR
	NAMES log/log.h
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Log include directory"
)

# MetaCall Log library path
find_library(METACALL_LOG_LIBRARY
	NAMES log
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Log library"
)

# MetaCall ADT include path
find_path(METACALL_ADT_INCLUDE_DIR
	NAMES adt/adt.h
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall ADT include directory"
)

# MetaCall ADT library path
find_library(METACALL_ADT_LIBRARY
	NAMES adt
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall ADT library"
)

# MetaCall Dynlink include path
find_path(METACALL_DYNLINK_INCLUDE_DIR
	NAMES dynlink/dynlink.h
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Dynlink include directory"
)

# MetaCall DynLink library path
find_library(METACALL_DYNLINK_LIBRARY
	NAMES dynlink
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall DynLink library"
)

# MetaCall Reflect include path
find_path(METACALL_REFLECT_INCLUDE_DIR
	NAMES reflect/reflect.h
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Reflect include directory"
)

# MetaCall Reflect library path
find_library(METACALL_REFLECT_LIBRARY
	NAMES reflect
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Reflect library"
)

# MetaCall Loader include path
find_path(METACALL_LOADER_INCLUDE_DIR
	NAMES loader/loader.h
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Loader include directory"
)

# MetaCall Loader library path
find_library(METACALL_LOADER_LIBRARY
	NAMES loader
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Loader library"
)

# MetaCall Core include path
find_path(METACALL_CORE_INCLUDE_DIR
	NAMES metacall/metacall.h
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Core include directory"
)

# MetaCall Core library path
find_library(METACALL_CORE_LIBRARY
	NAMES metacall
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall Core library"
)

set(METACALL_INCLUDE_DIR
    #${METACALL_VERSION_INCLUDE_DIR}
    ${METACALL_PREPROCESSOR_INCLUDE_DIR}
    ${METACALL_FORMAT_INCLUDE_DIR}
    ${METACALL_LOG_INCLUDE_DIR}
    ${METACALL_ADT_INCLUDE_DIR}
    ${METACALL_DYNLINK_INCLUDE_DIR}
    ${METACALL_REFLECT_INCLUDE_DIR}
    ${METACALL_LOADER_INCLUDE_DIR}
    ${METACALL_CORE_INCLUDE_DIR}
)

set(METACALL_LIBRARY
    ${METACALL_VERSION_LIBRARY}
    ${METACALL_PREPROCESSOR_LIBRARY}
    ${METACALL_FORMAT_LIBRARY}
    ${METACALL_LOG_LIBRARY}
    ${METACALL_ADT_LIBRARY}
    ${METACALL_DYNLINK_LIBRARY}
    ${METACALL_REFLECT_LIBRARY}
    ${METACALL_LOADER_LIBRARY}
    ${METACALL_CORE_LIBRARY}
)

# Find package
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(METACALL DEFAULT_MSG METACALL_INCLUDE_DIR METACALL_LIBRARY)

if(METACALL_FOUND)
	set(METACALL_LIBRARIES ${METACALL_LIBRARY})
	set(METACALL_INCLUDE_DIRS ${METACALL_INCLUDE_DIR})
	set(METACALL_DEFINITIONS)
endif()

# Ignore local variables
mark_as_advanced(METACALL_ROOT_DIR METACALL_INCLUDE_DIR METACALL_LIBRARY)
