#
# Cross-platform dynamic load library support by Parra Studios
# Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Cross-platform dynamic loading library utility
#

if(DYNAMICLOADING_FOUND)
	return()
endif()

set(DYNAMICLOADING_FOUND YES)

# Include portability module
include(Portability)

# Test against linux
if(PROJECT_OS_LINUX)

	include(FindLibDL)

	set(DYNAMIC_LOADING_INCLUDE_DIR ${LIBDL_INCLUDE_DIR})
	set(DYNAMIC_LOADING_LIBRARY ${LIBDL_LIBRARIES})

endif()

# TODO: Other platforms
