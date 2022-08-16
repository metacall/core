set(STACK_WALKING_UNWIND TRUE CACHE BOOL
	"Use compiler's unwind API")
set(STACK_WALKING_BACKTRACE FALSE CACHE BOOL
	"Use backtrace from (e)glibc for stack walking")
set(STACK_WALKING_LIBUNWIND FALSE CACHE BOOL
	"Use libunwind for stack walking")

set(STACK_DETAILS_AUTO_DETECT TRUE CACHE BOOL
	"Auto detect backward's stack details dependencies")

set(STACK_DETAILS_BACKTRACE_SYMBOL FALSE CACHE BOOL
	"Use backtrace from (e)glibc for symbols resolution")
set(STACK_DETAILS_DW FALSE CACHE BOOL
	"Use libdw to read debug info")
set(STACK_DETAILS_BFD FALSE CACHE BOOL
	"Use libbfd to read debug info")
set(STACK_DETAILS_DWARF FALSE CACHE BOOL
	"Use libdwarf/libelf to read debug info")
###############################################################################
# CONFIGS
###############################################################################
include(FindPackageHandleStandardArgs)

if (STACK_WALKING_LIBUNWIND)
	# libunwind works on the macOS without having to add special include
	# paths or libraries
	if (NOT APPLE)
		find_path(LIBUNWIND_INCLUDE_DIR NAMES "libunwind.h")
		find_library(LIBUNWIND_LIBRARY unwind)

		if (LIBUNWIND_LIBRARY)
			include(CheckSymbolExists)
			check_symbol_exists(UNW_INIT_SIGNAL_FRAME libunwind.h HAVE_UNW_INIT_SIGNAL_FRAME)
			if (NOT HAVE_UNW_INIT_SIGNAL_FRAME)
				message(STATUS "libunwind does not support unwinding from signal handler frames")
			endif()
		endif()
		if(LIBUNWIND_INCLUDE_DIR AND LIBUNWIND_LIBRARY)
			set(LIBUNWIND_INCLUDE_DIRS ${LIBUNWIND_INCLUDE_DIR})
			set(LIBUNWIND_LIBRARIES ${LIBUNWIND_LIBRARY})
			mark_as_advanced(LIBUNWIND_INCLUDE_DIRS LIBUNWIND_LIBRARIES)
		else ()
			set(BACKWARD_FOUND False)
			return()
		endif()
	endif()

	# Disable other unwinders if libunwind is found
	set(STACK_WALKING_UNWIND FALSE)
	set(STACK_WALKING_BACKTRACE FALSE)
endif()

if (${STACK_DETAILS_AUTO_DETECT})
	if(NOT CMAKE_VERSION VERSION_LESS 3.17)
		set(_name_mismatched_arg NAME_MISMATCHED)
	endif()
	# find libdw
	find_path(LIBDW_INCLUDE_DIR NAMES "elfutils/libdw.h" "elfutils/libdwfl.h")
	find_library(LIBDW_LIBRARY dw)
	# in case it's statically linked, look for all the possible dependencies
	find_library(LIBELF_LIBRARY elf)
	find_library(LIBPTHREAD_LIBRARY pthread)
	find_library(LIBZ_LIBRARY z)
	find_library(LIBBZ2_LIBRARY bz2)
	find_library(LIBLZMA_LIBRARY lzma)
	find_library(LIBZSTD_LIBRARY zstd)
	set(LIBDW_INCLUDE_DIRS ${LIBDW_INCLUDE_DIR} )
	set(LIBDW_LIBRARIES ${LIBDW_LIBRARY}
		$<$<BOOL:${LIBELF_LIBRARY}>:${LIBELF_LIBRARY}>
		$<$<BOOL:${LIBPTHREAD_LIBRARY}>:${LIBPTHREAD_LIBRARY}>
		$<$<BOOL:${LIBZ_LIBRARY}>:${LIBZ_LIBRARY}>
		$<$<BOOL:${LIBBZ2_LIBRARY}>:${LIBBZ2_LIBRARY}>
		$<$<BOOL:${LIBLZMA_LIBRARY}>:${LIBLZMA_LIBRARY}>
		$<$<BOOL:${LIBZSTD_LIBRARY}>:${LIBZSTD_LIBRARY}>)
	find_package_handle_standard_args(libdw ${_name_mismatched_arg}
		REQUIRED_VARS LIBDW_LIBRARY LIBDW_INCLUDE_DIR)
	mark_as_advanced(LIBDW_INCLUDE_DIR LIBDW_LIBRARY)

	# find libbfd
	find_path(LIBBFD_INCLUDE_DIR NAMES "bfd.h")
	find_path(LIBDL_INCLUDE_DIR NAMES "dlfcn.h")
	find_library(LIBBFD_LIBRARY bfd)
	find_library(LIBDL_LIBRARY dl)
	set(LIBBFD_INCLUDE_DIRS ${LIBBFD_INCLUDE_DIR} ${LIBDL_INCLUDE_DIR})
	set(LIBBFD_LIBRARIES ${LIBBFD_LIBRARY} ${LIBDL_LIBRARY})
	find_package_handle_standard_args(libbfd ${_name_mismatched_arg}
		REQUIRED_VARS LIBBFD_LIBRARY LIBBFD_INCLUDE_DIR
		LIBDL_LIBRARY LIBDL_INCLUDE_DIR)
	mark_as_advanced(LIBBFD_INCLUDE_DIR LIBBFD_LIBRARY
		LIBDL_INCLUDE_DIR LIBDL_LIBRARY)

	# find libdwarf
	find_path(LIBDWARF_INCLUDE_DIR NAMES "libdwarf.h" PATH_SUFFIXES libdwarf)
	find_path(LIBELF_INCLUDE_DIR NAMES "libelf.h")
	find_path(LIBDL_INCLUDE_DIR NAMES "dlfcn.h")
	find_library(LIBDWARF_LIBRARY dwarf)
	find_library(LIBELF_LIBRARY elf)
	find_library(LIBDL_LIBRARY dl)
	set(LIBDWARF_INCLUDE_DIRS ${LIBDWARF_INCLUDE_DIR} ${LIBELF_INCLUDE_DIR} ${LIBDL_INCLUDE_DIR})
	set(LIBDWARF_LIBRARIES ${LIBDWARF_LIBRARY} ${LIBELF_LIBRARY} ${LIBDL_LIBRARY})
	find_package_handle_standard_args(libdwarf ${_name_mismatched_arg}
		REQUIRED_VARS LIBDWARF_LIBRARY LIBDWARF_INCLUDE_DIR
		LIBELF_LIBRARY LIBELF_INCLUDE_DIR
		LIBDL_LIBRARY LIBDL_INCLUDE_DIR)
	mark_as_advanced(LIBDWARF_INCLUDE_DIR LIBDWARF_LIBRARY
		LIBELF_INCLUDE_DIR LIBELF_LIBRARY
		LIBDL_INCLUDE_DIR LIBDL_LIBRARY)

	if (LIBDW_FOUND)
		LIST(APPEND _BACKWARD_INCLUDE_DIRS ${LIBDW_INCLUDE_DIRS})
		LIST(APPEND _BACKWARD_LIBRARIES ${LIBDW_LIBRARIES})
		set(STACK_DETAILS_DW TRUE)
		set(STACK_DETAILS_BFD FALSE)
		set(STACK_DETAILS_DWARF FALSE)
		set(STACK_DETAILS_BACKTRACE_SYMBOL FALSE)
	elseif(LIBBFD_FOUND)
		LIST(APPEND _BACKWARD_INCLUDE_DIRS ${LIBBFD_INCLUDE_DIRS})
		LIST(APPEND _BACKWARD_LIBRARIES ${LIBBFD_LIBRARIES})

		# If we attempt to link against static bfd, make sure to link its dependencies, too
		get_filename_component(bfd_lib_ext "${LIBBFD_LIBRARY}" EXT)
		if (bfd_lib_ext STREQUAL "${CMAKE_STATIC_LIBRARY_SUFFIX}")
			list(APPEND _BACKWARD_LIBRARIES iberty z)
		endif()

		set(STACK_DETAILS_DW FALSE)
		set(STACK_DETAILS_BFD TRUE)
		set(STACK_DETAILS_DWARF FALSE)
		set(STACK_DETAILS_BACKTRACE_SYMBOL FALSE)
	elseif(LIBDWARF_FOUND)
		LIST(APPEND _BACKWARD_INCLUDE_DIRS ${LIBDWARF_INCLUDE_DIRS})
		LIST(APPEND _BACKWARD_LIBRARIES ${LIBDWARF_LIBRARIES})

		set(STACK_DETAILS_DW FALSE)
		set(STACK_DETAILS_BFD FALSE)
		set(STACK_DETAILS_DWARF TRUE)
		set(STACK_DETAILS_BACKTRACE_SYMBOL FALSE)
	else()
		set(STACK_DETAILS_DW FALSE)
		set(STACK_DETAILS_BFD FALSE)
		set(STACK_DETAILS_DWARF FALSE)
		set(STACK_DETAILS_BACKTRACE_SYMBOL TRUE)
	endif()
else()
	if (STACK_DETAILS_DW)
		LIST(APPEND _BACKWARD_LIBRARIES dw)
	endif()

	if (STACK_DETAILS_BFD)
		LIST(APPEND _BACKWARD_LIBRARIES bfd dl)
	endif()

	if (STACK_DETAILS_DWARF)
		LIST(APPEND _BACKWARD_LIBRARIES dwarf elf)
	endif()
endif()

macro(map_definitions var_prefix define_prefix)
	foreach(def ${ARGN})
		if (${${var_prefix}${def}})
			LIST(APPEND _BACKWARD_DEFINITIONS "${define_prefix}${def}=1")
		else()
			LIST(APPEND _BACKWARD_DEFINITIONS "${define_prefix}${def}=0")
		endif()
	endforeach()
endmacro()

if (NOT _BACKWARD_DEFINITIONS)
	map_definitions("STACK_WALKING_" "BACKWARD_HAS_" UNWIND LIBUNWIND BACKTRACE)
	map_definitions("STACK_DETAILS_" "BACKWARD_HAS_" BACKTRACE_SYMBOL DW BFD DWARF)
endif()

if(WIN32)
    list(APPEND _BACKWARD_LIBRARIES dbghelp psapi)
	if(MINGW)
	    set(MINGW_MSVCR_LIBRARY "msvcr90$<$<CONFIG:DEBUG>:d>" CACHE STRING "Mingw MSVC runtime import library")
	    list(APPEND _BACKWARD_LIBRARIES ${MINGW_MSVCR_LIBRARY})
	endif()
endif()

set(BACKWARD_INCLUDE_LIBSDIR ${_BACKWARD_INCLUDE_DIRS})
set(BACKWARD_INCLUDE_DIR "${backward-cpp_SOURCE_DIR}")
set(BACKWARD_HAS_EXTERNAL_LIBRARIES FALSE)

find_path(BACKWARD_INCLUDE_DIRS
	NAMES backward.hpp
	PATHS BACKWARD_INCLUDE_DIR
)

if(NOT BACKWARD_INCLUDE_DIRS)
	set(BACKWARD_FOUND FALSE)
	return()
else()
	set(BACKWARD_FOUND TRUE)
endif()

set(BACKWARD_DEFINITIONS ${_BACKWARD_DEFINITIONS} CACHE INTERNAL "BACKWARD_DEFINITIONS")
set(BACKWARD_LIBRARIES ${_BACKWARD_LIBRARIES} CACHE INTERNAL "BACKWARD_LIBRARIES")
mark_as_advanced(BACKWARD_INCLUDE_LIBSDIR BACKWARD_INCLUDE_DIRS BACKWARD_DEFINITIONS BACKWARD_LIBRARIES)


foreach(var ${BACKWARD_DEFINITIONS})
  string(REPLACE "=" ";" var_as_list ${var})
  list(GET var_as_list 0 var_name)
  list(GET var_as_list 1 var_value)
  set(${var_name} ${var_value})
  mark_as_advanced(${var_name})
endforeach()
