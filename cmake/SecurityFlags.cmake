#
# Compiler and linker options for hardening flags by Parra Studios
# Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Enables hardening security flags if available
#

include(CheckCCompilerFlag)
include(CheckCCompilerFlagStackSmashing)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")

    # Detect position independent code flag
    check_c_compiler_flag("-fPIC" PIC_C_FLAG)

    if(PIC_C_FLAG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    endif()

    # Detect stack protector
    check_c_compiler_flag_stack_smashing("-fstack-protector" STACK_PROTECTOR_C_FLAG)

    if(STACK_PROTECTOR_C_FLAG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector")

            # use ssp-buffer-size if it is supported
            if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 4.9)
                set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --param ssp-buffer-size=4")
            endif()

    endif()

    # Detect fortify source
    check_c_compiler_flag("-D_FORTIFY_SOURCE=2" FORTIFY_SOURCE_C_FLAG)

    if(FORTIFY_SOURCE_C_FLAG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -D_FORTIFY_SOURCE=2")
    endif()

endif()

include(CheckCXXCompilerFlag)
include(CheckCXXCompilerFlagStackSmashing)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

    # Detect position independent code flag
    check_cxx_compiler_flag("-fPIC" PIC_CXX_FLAG)

    if(PIC_CXX_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    endif()

    # Detect stack protector
    check_cxx_compiler_flag_stack_smashing("-fstack-protector" STACK_PROTECTOR_CXX_FLAG)

    if(STACK_PROTECTOR_CXX_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")

            # use ssp-buffer-size if it is supported
            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
                set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --param ssp-buffer-size=4")
            endif()

    endif()

    # Detect fortify source
    check_cxx_compiler_flag("-D_FORTIFY_SOURCE=2" FORTIFY_SOURCE_CXX_FLAG)

    if(FORTIFY_SOURCE_CXX_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -D_FORTIFY_SOURCE=2")
    endif()

endif()

