# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_JAVA)
	return()
endif()

#
# External dependencies
#

find_package(Java)

if(NOT JAVA_FOUND)
	message(SEND_ERROR "Java libraries not found")
	return()
endif()

find_package(JNI)

if(NOT JNI_FOUND)
	message(SEND_ERROR "JNI libraries not found")
	return()
endif()

include(UseJava)

find_package(Maven)

if(NOT MAVEN_FOUND)
	message(SEND_ERROR "Maven not found")
	return()
endif()

#
# Port name and options
#

# Target name
set(target java_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

# TODO:
# https://cmake.org/cmake/help/latest/module/UseJava.html
# https://searchcode.com/codesearch/view/7927641/
# https://github.com/Kurento/kms-core/blob/master/CMake/FindMaven.cmake
