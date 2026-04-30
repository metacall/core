#
#	CMake PatchRPath by Parra Studios
#	Unified RPath patching for ELF (patchelf) and Mach-O (install_name_tool).
#
#	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

include_guard(GLOBAL)

# metacall_patch_rpath(TARGET_NAME LIBRARIES OUTPUT_DIR)
#
# Attach POST_BUILD commands to TARGET_NAME that fix the rpath of each library
# in LIBRARIES so sibling libraries resolve relative to the binary's location.
#
# On macOS:  install_name_tool -id @rpath/<basename> && -add_rpath @loader_path
# On Linux:  patchelf --set-rpath $ORIGIN

function(metacall_patch_rpath TARGET_NAME LIBRARIES OUTPUT_DIR)
	if(APPLE)
		find_program(INSTALL_NAME_TOOL_EXECUTABLE NAMES install_name_tool)

		if(NOT INSTALL_NAME_TOOL_EXECUTABLE)
			message(WARNING "install_name_tool not found, skipping Mach-O rpath patching for ${TARGET_NAME}")
			return()
		endif()

		foreach(lib_path ${LIBRARIES})
			get_filename_component(lib_basename "${lib_path}" NAME)

			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
				COMMAND "${INSTALL_NAME_TOOL_EXECUTABLE}" -id "@rpath/${lib_basename}" "${lib_path}"
				COMMAND "${INSTALL_NAME_TOOL_EXECUTABLE}" -add_rpath @loader_path "${lib_path}"
				COMMENT "Applying Mach-O rpath fixes for ${lib_basename}"
			)
		endforeach()

	elseif(PROJECT_OS_FAMILY MATCHES "unix")
		find_package(Patchelf)

		if(NOT Patchelf_FOUND)
			message(WARNING "patchelf not found, skipping ELF rpath patching for ${TARGET_NAME}")
			return()
		endif()

		foreach(lib_path ${LIBRARIES})
			get_filename_component(lib_basename "${lib_path}" NAME)

			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
				COMMAND "${Patchelf_EXECUTABLE}" --set-rpath "$ORIGIN" "${lib_path}"
				COMMENT "Applying ELF rpath fixes for ${lib_basename}"
			)
		endforeach()
	endif()
endfunction()
