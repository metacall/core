# Include cross-platform dynamic load library support

# Include portability module
include(Portability)

# Test against linux
if(PROJECT_OS_LINUX)

    include(FindLibDL)

    set(DYNAMIC_LOADING_INCLUDE_DIR ${LIBDL_INCLUDE_DIR})
    set(DYNAMIC_LOADING_LIBRARY ${LIBDL_LIBRARY})

endif()

# TODO: Other platforms
