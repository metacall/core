#
# External dependencies
#

find_package(DotNET)

if(NOT DOTNET_FOUND)
	message(SEND_ERROR "DotNET command not found")
	return()
endif()

if(DOTNET_VERSION VERSION_LESS "5.0")
	return()
endif()

#
# Configure csharp project
#

cs_project(function 0.1.0)
