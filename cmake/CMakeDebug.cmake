#
# CMake Debug utilities by Parra Studios
# Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# CMake debugging utilities and inspection facilities
#

if(CMAKE_DEBUG_FOUND)
	return()
endif()

set(CMAKE_DEBUG_FOUND YES)

execute_process(
	COMMAND ${CMAKE_COMMAND} --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST
)

string(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
string(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")

list(REMOVE_DUPLICATES CMAKE_PROPERTY_LIST)

function(cmake_debug_properties_print)
	message(STATUS "CMake Property List:")

	foreach(property ${CMAKE_PROPERTY_LIST})
		message(STATUS "- ${property}")
	endforeach()
endfunction()

function(cmake_debug_properties_target_print target)
	if(NOT TARGET ${target})
		message(STATUS "CMakeDebug: Invalid target ${target}")
		return()
	endif()

	string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_NAME)

	foreach(property ${CMAKE_PROPERTY_LIST})
		string(REPLACE "<CONFIG>" "${BUILD_TYPE_NAME}" property ${property})

		string(FIND "${property}" "LOCATION" POSITION)

		if(${POSITION} EQUAL -1)
			get_property(valid
				TARGET ${target}
				PROPERTY ${property}
				SET
			)

			if(valid)
				get_target_property(value ${target} ${property})

				if(NOT "${value}" STREQUAL "value-NOTFOUND")
					message(STATUS "${property} = ${value}")
				endif()
			endif()
		endif()

	endforeach()

endfunction()
