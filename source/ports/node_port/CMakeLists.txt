
#
# External dependencies
#

find_package(NodeJS 10.15.3)

if(NOT NODEJS_FOUND)
	message(STATUS "NodeJS libraries not found")
	return()
endif()

# MetaCall Library
find_package(MetaCall REQUIRED)

if(NOT METACALL_FOUND)
	message(STATUS "MetaCall libraries not found")
	return()
endif()

# TODO
