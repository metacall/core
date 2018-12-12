#
#	CMake Distributable (Unity Build) library by Parra Studios
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

function(distributable_generate target_root unity_build_file)

	set(target_list "${ARGN}")

	set(unity_build_source)

	foreach(target ${target_list})

		# Get target source files
		get_target_property(target_sources
			${META_PROJECT_NAME}::${target}
			SOURCES
		)

		# Add private linkage for all targets except metacall
		if(NOT "${target}" STREQUAL "${target_root}")
			string(TOUPPER ${target} target_upper)
			set(unity_build_source "${unity_build_source}\n\#ifndef ${target_upper}_API")

			if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
				set(unity_build_source "${unity_build_source}\n\#	define ${target_upper}_API __attribute__ ((visibility(\"hidden\")))")
			else()
				set(unity_build_source "${unity_build_source}\n\#	define ${target_upper}_API ${target_upper}_NO_EXPORT")
				add_definitions("-D${target_upper}_API=${target_upper}_NO_EXPORT")
			endif()

			set(unity_build_source "${unity_build_source}\n\#endif /* ${target_upper}_API */")
		else()
			set(unity_build_source "${unity_build_source}\n\#ifndef ${target}_EXPORTS")
			set(unity_build_source "${unity_build_source}\n\#	define ${target}_EXPORTS")
			set(unity_build_source "${unity_build_source}\n\#endif /* ${target}_EXPORTS */")

			add_definitions("-D${target}_EXPORTS")
		endif()

		# Add include paths
		include_directories("${CMAKE_BINARY_DIR}/source/${target}/include")
		include_directories("${CMAKE_SOURCE_DIR}/source/${target}/include")

		# Write auto-generated includes into unity build
		set(unity_build_source "${unity_build_source}\n\#include <${CMAKE_BINARY_DIR}/source/${target}/include/${target}/${target}_api.h>")
		set(unity_build_source "${unity_build_source}\n\#include <${CMAKE_BINARY_DIR}/source/${target}/include/${target}/${target}_features.h>")

		# Write all includes into unity build
		foreach(source ${target_sources})
			set(unity_build_source "${unity_build_source}\n\#include <${source}>")
		endforeach()

	endforeach()

	if(EXISTS "${unity_build_file}")
		file(READ "${unity_build_file}" unity_build_source_old)

		if("${unity_build_source_old}" STREQUAL "${unity_build_source}")
			return()
		endif()
	endif()

	file(WRITE ${unity_build_file} "${unity_build_source}")

	message(STATUS "Unity build written in ${unity_build_file}")

endfunction()
