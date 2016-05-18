#
# Portability CMake support by Parra Studios
# Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Cross-platform and architecture detection utility
#

if(PORTABILITY_FOUND)
    return()
endif()

set(PORTABILITY_FOUND YES)

#
# Check the OS type
#

# Check Linux
string(REGEX MATCH "Linux" PROJECT_OS_LINUX ${CMAKE_SYSTEM_NAME})

if(PROJECT_OS_LINUX)
    set(PROJECT_OS_LINUX TRUE BOOL INTERNAL)
    set(PROJECT_OS_NAME "Linux")
    set(PROJECT_OS_FAMILY unix)
endif()

# Check BSD
string(REGEX MATCH "BSD" PROJECT_OS_BSD ${CMAKE_SYSTEM_NAME})

if(PROJECT_OS_BSD)
    set(PROJECT_OS_BSD TRUE BOOL INTERNAL)
    set(PROJECT_OS_NAME "BSD")
    set(PROJECT_OS_FAMILY unix)
endif()

# Check Solaris
string(REGEX MATCH "SunOS" PROJECT_OS_SOLARIS ${CMAKE_SYSTEM_NAME})

if(PROJECT_OS_SOLARIS)
    set(PROJECT_OS_SOLARIS TRUE BOOL INTERNAL)
    set(PROJECT_OS_NAME "Solaris")
    set(PROJECT_OS_FAMILY unix)
endif()

# Check Windows
if(WIN32)
    set(PROJECT_OS_WIN TRUE BOOL INTERNAL)
    set(PROJECT_OS_NAME "Windows")
    set(PROJECT_OS_FAMILY win32)
endif()

# Check Apple OS
if(APPLE)
    # Check if it's OS X or another MacOS
    string(REGEX MATCH "Darwin" PROJECT_OS_OSX ${CMAKE_SYSTEM_NAME})

    if(NOT PROJECT_OS_OSX)
        set(PROJECT_OS_MACOS TRUE BOOL INTERNAL)
        set(PROJECT_OS_NAME "Apple MacOS")
    else()
        set(PROJECT_OS_MACOSX TRUE BOOL INTERNAL)
        set(PROJECT_OS_NAME "Apple MacOS X")
    endif()

    set(PROJECT_OS_FAMILY macos)
endif()

# Check QNX
if(QNXNTO)
    set(PROJECT_OS_QNX TRUE BOOL INTERNAL)
    set(PROJECT_OS_NAME "QNX")
    set(PROJECT_OS_FAMILY qnx)
endif()

# Check UNIX
if(NOT PROJECT_OS_NAME AND UNIX)
    set(PROJECT_OS_UNIX TRUE BOOL INTERNAL)
    set(PROJECT_OS_NAME "UNIX")
    set(PROJECT_OS_FAMILY unix)
endif()

message(STATUS "Target Operative System: ${PROJECT_OS_NAME}")
message(STATUS "Target OS Family: ${PROJECT_OS_FAMILY}")

# 32 or 64 bit Linux
#if(PROJECT_OS_LINUX)
#    # Set the library directory suffix accordingly
#    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
#        set(PROJECT_PROC_64BIT TRUE BOOL INTERNAL)
#        message(STATUS "Linux x86_64 Detected")
#    elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ppc64")
#        message(STATUS "Linux ppc64 Detected")
#        set(PROJECT_PROC_64BIT TRUE BOOL INTERNAL)
#    elseif($CMAKE_SYSTEM_PROCESSOR} STREQUAL "s390x")
#	message(STATEUS "Linux s390x Detected")
#	set(PROJECT_PROC_64BIT TRUE BOOL INTERNAL)
#    endif()
#endif()
#
#if(PROJECT_PROC_64BIT)
#    # Set the install path to lib64
#    set(PROJECT_LIB_DIR "lib64")
#    set(PROJECT_PLUGIN_DIR "lib64/${PROJECT_NAME}-${META_VERSION}")
#else(PROJECT_PROC_64BIT)
#    set(PROJECT_LIB_DIR "lib")
#    set(PROJECT_PLUGIN_DIR "lib/${PROJECT_NAME}-${META_VERSION}")
#endif()
#
#message(STATUS "Installing Libraries to ${CMAKE_INSTALL_PREFIX}/${PROJECT_LIB_DIR}")
#message(STATUS "Installing Plugins to ${CMAKE_INSTALL_PREFIX}/${PROJECT_PLUGIN_DIR}")
