#
# Code formatting (clang-format) and line endings check
#
# This file can be used in two ways:
#
# 1) Included from the root CMakeLists.txt, it defines the targets:
#	clang-format        Format the C/C++ sources in place
#	clang-format-check  Check the formatting of the C/C++ sources (same check as the CI)
#	eol-check           Check that no tracked file is stored with CRLF line endings
#
# 2) Executed as a script, it does not require a configured build:
#	cmake -DMODE=<fix|check|eol> [-DFILES=<file;...>] -P cmake/Format.cmake
#
#	MODE                    fix: format the files in place
#	                        check: fail if any file is not formatted
#	                        eol: fail if any tracked file is stored with CRLF line endings
#	FILES                   Optional list of files, absolute or relative to the repository root,
#	                        by default all the tracked C/C++ files under source/ (fix and check)
#	                        or all the tracked files (eol)
#	ClangFormat_EXECUTABLE  Optional path to the clang-format executable
#
# The clang-format version is fixed (FORMAT_CLANG_FORMAT_VERSION) because
# each version formats differently. If it is not found, the formatting runs
# through docker or podman, with the same image used by the CI.
#
# When updating the clang-format version, update also:
#	tools/metacall-environment.sh and tools/metacall-environment.ps1 (clangformat option)
#	AGENTS.md, docs/README.md and source/loaders/lua_loader/README.md
#

# In script mode there is no project setting the policies (i.e: IN_LIST)
if(CMAKE_SCRIPT_MODE_FILE)
	cmake_minimum_required(VERSION 3.15)
endif()

set(FORMAT_CLANG_FORMAT_VERSION 19)
set(FORMAT_CONTAINER_IMAGE "ghcr.io/jidicula/clang-format:${FORMAT_CLANG_FORMAT_VERSION}")
set(FORMAT_SOURCE_DIRECTORY "source")
set(FORMAT_EXTENSIONS c cc cpp cxx h hh hpp hxx inl)

# Number of files passed to each clang-format execution, it keeps the
# command line under the Windows limit (32767 characters)
set(FORMAT_BATCH_SIZE 100)

# Repository root, this file is located in <root>/cmake
get_filename_component(FORMAT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

#
# Find the formatting tool
#

# Obtains the command used for running clang-format in _result and a description
# of it in _description, or an empty command if none is available. It uses the
# local clang-format if it has the required version, otherwise docker or podman.
function(format_find_tool _result _description _readonly)

	list(APPEND CMAKE_MODULE_PATH "${FORMAT_ROOT}/cmake")

	# EXACT accepts only the same major version (see cmake/FindClangFormat.cmake)
	find_package(ClangFormat ${FORMAT_CLANG_FORMAT_VERSION} EXACT QUIET)

	if(ClangFormat_FOUND)
		set(${_result} "${ClangFormat_EXECUTABLE}" PARENT_SCOPE)
		set(${_description} "clang-format ${ClangFormat_VERSION} (${ClangFormat_EXECUTABLE})" PARENT_SCOPE)
		return()
	endif()

	if(ClangFormat_EXECUTABLE AND ClangFormat_VERSION)
		set(found_description "clang-format ${ClangFormat_VERSION} found at ${ClangFormat_EXECUTABLE} but version ${FORMAT_CLANG_FORMAT_VERSION} is required")
	else()
		set(found_description "clang-format ${FORMAT_CLANG_FORMAT_VERSION} not found")
	endif()

	find_program(FORMAT_CONTAINER_EXECUTABLE NAMES docker podman)

	if(NOT FORMAT_CONTAINER_EXECUTABLE)
		set(${_result} "" PARENT_SCOPE)
		set(${_description} "${found_description}, and neither docker nor podman were found" PARENT_SCOPE)
		return()
	endif()

	set(mount "type=bind,source=${FORMAT_ROOT},target=/src")

	if(_readonly)
		set(mount "${mount},readonly")
	endif()

	set(command "${FORMAT_CONTAINER_EXECUTABLE}" run --rm --mount "${mount}" --workdir /src)

	# Run as the current user, so the formatted files keep their owner
	get_filename_component(container_name "${FORMAT_CONTAINER_EXECUTABLE}" NAME_WE)

	if(container_name STREQUAL "podman")
		# Rootless podman maps the container user into a subordinate range, keep-id maps it to the current user
		list(APPEND command --userns=keep-id)
	elseif(UNIX AND NOT APPLE)
		execute_process(COMMAND id -u OUTPUT_VARIABLE uid OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE uid_result ERROR_QUIET)
		execute_process(COMMAND id -g OUTPUT_VARIABLE gid OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE gid_result ERROR_QUIET)
		execute_process(COMMAND id -G OUTPUT_VARIABLE groups OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE groups_result ERROR_QUIET)

		if(uid_result EQUAL 0 AND gid_result EQUAL 0)
			list(APPEND command --user "${uid}:${gid}")

			# Keep the supplementary groups, the files may be accessible only through them
			if(groups_result EQUAL 0)
				string(REPLACE " " ";" groups "${groups}")

				foreach(group IN LISTS groups)
					if(NOT group STREQUAL gid)
						list(APPEND command --group-add "${group}")
					endif()
				endforeach()
			endif()
		endif()
	endif()

	list(APPEND command "${FORMAT_CONTAINER_IMAGE}")

	set(${_result} "${command}" PARENT_SCOPE)
	set(${_description} "${found_description}, using ${FORMAT_CONTAINER_EXECUTABLE} with ${FORMAT_CONTAINER_IMAGE}" PARENT_SCOPE)

endfunction()

#
# Targets (included from CMakeLists.txt)
#

if(NOT CMAKE_SCRIPT_MODE_FILE)

	format_find_tool(format_tool format_tool_description FALSE)

	if(format_tool)
		message(STATUS "Formatting: ${format_tool_description}")
	else()
		message(STATUS "Formatting: ${format_tool_description}, the clang-format targets will fail")
	endif()

	add_custom_target(clang-format
		COMMAND ${CMAKE_COMMAND} -DMODE=fix -P "${CMAKE_CURRENT_LIST_FILE}"
		COMMENT "Formatting C/C++ sources with clang-format ${FORMAT_CLANG_FORMAT_VERSION}"
		USES_TERMINAL
		VERBATIM
	)

	add_custom_target(clang-format-check
		COMMAND ${CMAKE_COMMAND} -DMODE=check -P "${CMAKE_CURRENT_LIST_FILE}"
		COMMENT "Checking C/C++ sources formatting with clang-format ${FORMAT_CLANG_FORMAT_VERSION}"
		USES_TERMINAL
		VERBATIM
	)

	add_custom_target(eol-check
		COMMAND ${CMAKE_COMMAND} -DMODE=eol -P "${CMAKE_CURRENT_LIST_FILE}"
		COMMENT "Checking line endings of tracked files"
		USES_TERMINAL
		VERBATIM
	)

	return()
endif()

#
# Script mode (cmake -DMODE=<fix|check|eol> -P cmake/Format.cmake)
#

if(NOT MODE MATCHES "^(fix|check|eol)$")
	message(FATAL_ERROR "Invalid MODE '${MODE}', use: cmake -DMODE=<fix|check|eol> [-DFILES=<file;...>] -P cmake/Format.cmake")
endif()

find_package(Git QUIET)

# Normalize the list of files passed in FILES to paths relative to the repository
# root, discarding the ones that do not exist (i.e: deleted files in a commit)
function(format_files_normalize _result _files)

	set(result "")

	foreach(file IN LISTS _files)
		get_filename_component(file_absolute "${file}" ABSOLUTE BASE_DIR "${FORMAT_ROOT}")
		file(RELATIVE_PATH file_relative "${FORMAT_ROOT}" "${file_absolute}")

		if(file_relative MATCHES "^\\.\\./")
			message(FATAL_ERROR "File '${file}' is outside of the repository '${FORMAT_ROOT}'")
		endif()

		if(EXISTS "${file_absolute}" AND NOT IS_DIRECTORY "${file_absolute}")
			list(APPEND result "${file_relative}")
		endif()
	endforeach()

	set(${_result} "${result}" PARENT_SCOPE)

endfunction()

#
# Line endings check
#

if(MODE STREQUAL "eol")

	if(NOT GIT_FOUND)
		message(FATAL_ERROR "The line endings check requires git")
	endif()

	set(pathspec "")

	if(DEFINED FILES)
		format_files_normalize(pathspec "${FILES}")

		if(NOT pathspec)
			message(STATUS "No files to check")
			return()
		endif()
	endif()

	# The index (i/) holds the content that is committed, the attributes and
	# the working tree do not matter, so the check is the same on every platform
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" -c core.quotepath=off ls-files --eol -- ${pathspec}
		WORKING_DIRECTORY "${FORMAT_ROOT}"
		OUTPUT_VARIABLE eol_output
		RESULT_VARIABLE eol_result
	)

	if(NOT eol_result EQUAL 0)
		message(FATAL_ERROR "Failed to list the line endings of the tracked files (git ls-files --eol)")
	endif()

	string(REPLACE "\n" ";" eol_lines "${eol_output}")

	set(eol_invalid "")

	foreach(line IN LISTS eol_lines)
		# Format: i/<eol> w/<eol> attr/<attributes> <TAB> <path>
		if(line MATCHES "^i/(crlf|mixed) +w/[^ ]* +attr/[^\t]*\t(.*)$")
			list(APPEND eol_invalid "${CMAKE_MATCH_1}\t${CMAKE_MATCH_2}")
		endif()
	endforeach()

	if(eol_invalid)
		list(LENGTH eol_invalid eol_invalid_length)
		string(REPLACE ";" "\n" eol_invalid_str "${eol_invalid}")
		message("${eol_invalid_str}")
		message(FATAL_ERROR "${eol_invalid_length} file(s) stored with CRLF or mixed line endings, they must use LF only")
	endif()

	message(STATUS "Line endings check passed")
	return()
endif()

#
# Formatting (fix and check)
#

set(files "")

if(DEFINED FILES)
	format_files_normalize(files_normalized "${FILES}")

	# Filter by directory and extension, the hook passes all the staged files,
	# so the same files are checked than when FILES is not defined (and in the CI)
	foreach(file IN LISTS files_normalized)
		get_filename_component(file_extension "${file}" LAST_EXT)
		string(SUBSTRING "${file_extension}" 1 -1 file_extension)

		if(file MATCHES "^${FORMAT_SOURCE_DIRECTORY}/" AND file_extension IN_LIST FORMAT_EXTENSIONS)
			list(APPEND files "${file}")
		endif()
	endforeach()
elseif(GIT_FOUND AND EXISTS "${FORMAT_ROOT}/.git")
	set(pathspec "")

	foreach(extension IN LISTS FORMAT_EXTENSIONS)
		list(APPEND pathspec "${FORMAT_SOURCE_DIRECTORY}/*.${extension}")
	endforeach()

	execute_process(
		COMMAND "${GIT_EXECUTABLE}" -c core.quotepath=off ls-files -- ${pathspec}
		WORKING_DIRECTORY "${FORMAT_ROOT}"
		OUTPUT_VARIABLE files_output
		RESULT_VARIABLE files_result
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	if(NOT files_result EQUAL 0)
		message(FATAL_ERROR "Failed to list the tracked files (git ls-files)")
	endif()

	string(REPLACE "\n" ";" files "${files_output}")
else()
	# Not a git repository (i.e: a source tarball)
	set(globs "")

	foreach(extension IN LISTS FORMAT_EXTENSIONS)
		list(APPEND globs "${FORMAT_SOURCE_DIRECTORY}/*.${extension}")
	endforeach()

	file(GLOB_RECURSE files LIST_DIRECTORIES FALSE RELATIVE "${FORMAT_ROOT}" ${globs})
endif()

if(NOT files)
	message(STATUS "No files to ${MODE}")
	return()
endif()

if(MODE STREQUAL "check")
	format_find_tool(format_tool format_tool_description TRUE)
	set(format_arguments --style=file --dry-run --Werror)
else()
	format_find_tool(format_tool format_tool_description FALSE)
	set(format_arguments --style=file -i)
endif()

if(NOT format_tool)
	message(FATAL_ERROR "${format_tool_description}. Install clang-format ${FORMAT_CLANG_FORMAT_VERSION} (tools/metacall-environment.sh clangformat), docker or podman")
endif()

list(LENGTH files files_length)
message(STATUS "Running ${MODE} on ${files_length} file(s) with ${format_tool_description}")

# Run in batches and report all the failures at the end
set(format_failed FALSE)
set(batch "")
set(batch_length 0)
set(index 0)

foreach(file IN LISTS files)
	list(APPEND batch "${file}")
	math(EXPR batch_length "${batch_length} + 1")
	math(EXPR index "${index} + 1")

	if(batch_length EQUAL FORMAT_BATCH_SIZE OR index EQUAL files_length)
		execute_process(
			COMMAND ${format_tool} ${format_arguments} ${batch}
			WORKING_DIRECTORY "${FORMAT_ROOT}"
			RESULT_VARIABLE format_result
		)

		if(NOT format_result EQUAL 0)
			set(format_failed TRUE)
		endif()

		set(batch "")
		set(batch_length 0)
	endif()
endforeach()

if(format_failed)
	if(MODE STREQUAL "check")
		if(DEFINED FILES)
			message(FATAL_ERROR "Formatting check failed, run: cmake -DMODE=fix \"-DFILES=${files}\" -P cmake/Format.cmake")
		else()
			message(FATAL_ERROR "Formatting check failed, run: cmake -DMODE=fix -P cmake/Format.cmake")
		endif()
	else()
		message(FATAL_ERROR "Formatting failed")
	endif()
endif()

message(STATUS "Formatting ${MODE} passed")
