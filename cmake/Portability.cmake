#
#	Portability CMake support by Parra Studios
#	Cross-platform and architecture detection utility.
#
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
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

#
# Check the Architecture type
#

set(PROJECT_ARCH_NAME ${CMAKE_SYSTEM_PROCESSOR})

# 32 or 64 bit Linux
if(PROJECT_OS_LINUX)
	if(${PROJECT_ARCH_NAME} STREQUAL "x86")
		set(PROJECT_ARCH_32BIT TRUE BOOL INTERNAL)
	endif()

	if(${PROJECT_ARCH_NAME} STREQUAL "x86_64")
		set(PROJECT_ARCH_64BIT TRUE BOOL INTERNAL)
	elseif(${PROJECT_ARCH_NAME} STREQUAL "ppc64")
		set(PROJECT_ARCH_64BIT TRUE BOOL INTERNAL)
	elseif(${PROJECT_ARCH_NAME} STREQUAL "s390x")
	set(PROJECT_ARCH_64BIT TRUE BOOL INTERNAL)
	endif()

	if(PROJECT_ARCH_32BIT)
		if("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
			message(STATUS "Linux ${PROJECT_ARCH_NAME} 32bit detected")
		else()
			message(WARNING "Linux ${PROJECT_ARCH_NAME} 32bit does not mach size of a pointer: ${CMAKE_SIZEOF_VOID_P}")
		endif()
	elseif(PROJECT_ARCH_64BIT)
		if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
			message(STATUS "Linux ${PROJECT_ARCH_NAME} 64bit detected")
		else()
			message(WARNING "Linux ${PROJECT_ARCH_NAME} 64bit does not mach size of a pointer: ${CMAKE_SIZEOF_VOID_P}")
		endif()
	else()
		message(STATUS "Linux ${PROJECT_ARCH_NAME} detected")
	endif()
endif()

# Detect architecture based on pointer size
if(NOT PROJECT_ARCH_32BIT AND NOT PROJECT_ARCH_64BIT)
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
		message(STATUS "32bit architecture ${PROJECT_ARCH_NAME} detected")
	set(PROJECT_ARCH_32BIT TRUE BOOL INTERNAL)

		if(PROJECT_OS_WIN)
			set(WINXBITS Win32)
		endif()

	elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		message(STATUS "64bit architecture ${PROJECT_ARCH_NAME} detected")
	set(PROJECT_ARCH_64BIT TRUE BOOL INTERNAL)

		if(PROJECT_OS_WIN)
			set(WINXBITS Win64)
		endif()

	else()
		 message(WARNING "Architecture detection error, invalid size of void pointer: ${CMAKE_SIZEOF_VOID_P}")
	endif()
endif()

# Set the library directory suffix accordingly
#if(PROJECT_PROC_64BIT)
#	# Set the install path to lib64
#	set(PROJECT_LIB_DIR "lib64")
#	set(PROJECT_PLUGIN_DIR "lib64/${PROJECT_NAME}-${META_VERSION}")
#else(PROJECT_PROC_64BIT)
#	set(PROJECT_LIB_DIR "lib")
#	set(PROJECT_PLUGIN_DIR "lib/${PROJECT_NAME}-${META_VERSION}")
#endif()

#message(STATUS "Installing Libraries to ${CMAKE_INSTALL_PREFIX}/${PROJECT_LIB_DIR}")
#message(STATUS "Installing Plugins to ${CMAKE_INSTALL_PREFIX}/${PROJECT_PLUGIN_DIR}")
