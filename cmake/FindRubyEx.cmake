#
# CMake Find Ruby Engine by Parra Studios
# Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Find Ruby library and include paths (Ruby 1.8, 1.9, 2.0, 2.1, 2.2, 2.3 and
# 2.4 are supported)
#
# RUBY_FOUND			- True if Ruby was found
# RUBY_INCLUDE_DIRS		- Ruby headers path
# RUBY_LIBRARY_DIRS		- List of Ruby library directories
# RUBY_LIBRARY			- List of Ruby libraries
# RUBY_VERSION			- Ruby version
# RUBY_VERSION_MAJOR	- Major Ruby version
# RUBY_VERSION_MINOR	- Minor Ruby version
# RUBY_VERSION_PATCH	- Patch Ruby version


if(RUBY_FOUND)
	set(RUBY_FIND_QUIETLY TRUE)
endif()

# Uncomment this option to debug
# set(_RUBY_CMAKE_DEBUG TRUE)

find_package(PkgConfig)

if(PKG_CONFIG_FOUND)
	set(RUBY_MODULE_NAMES
		ruby-2.4 ruby-2.3 ruby-2.2 ruby-2.1 ruby-2.0 ruby-1.9 ruby-1.8
	)

	pkg_search_module(RUBY
		${RUBY_MODULE_NAMES}
	)
endif()


if(RUBY_FOUND)

	if(RUBY_EXECUTABLE)
		execute_process(
			COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['MAJOR']"
			OUTPUT_VARIABLE RUBY_VERSION_MAJOR
		)

		execute_process(
			COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['MINOR']"
			OUTPUT_VARIABLE RUBY_VERSION_MINOR
		)

		execute_process(
			COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['TEENY']"
			OUTPUT_VARIABLE RUBY_VERSION_PATCH
		)

		set(RUBY_VERSION "${RUBY_VERSION_MAJOR}.${RUBY_VERSION_MINOR}.${RUBY_VERSION_PATCH}")
	endif()

	# Package find is broken, library must be set manually

	# TODO: Implement regex match
	#if()
	#	message(FATAL_ERROR "RUBY_LIBRARY (${RUBY_LIBRARY}) does not match current ruby version (${RUBY_VERSION}), it must be set from CMake options.")
	#endif()

else()

	set(RUBY_EXECUTABLE_NAMES
		ruby2.4.0 ruby240 ruby2.4 ruby2.3.0 ruby230 ruby2.3 ruby23 ruby2.2.3 ruby223
		ruby2.2.2 ruby222 ruby2.2.1 ruby221 ruby2.2.0 ruby220 ruby2.2 ruby22 ruby2.1.7 ruby217
		ruby2.1.6 ruby216 ruby2.1.5 ruby215 ruby2.1.4 ruby214 ruby2.1.3 ruby213
		ruby2.1.2 ruby212 ruby2.1.1 ruby211 ruby2.1.0 ruby210 ruby2.1 ruby21
		ruby2.0 ruby20 ruby1.9.3 ruby193 ruby1.9.2 ruby192 ruby1.9.1 ruby191 ruby1.9 ruby19
		ruby1.8 ruby18 ruby
	)

	find_program(RUBY_EXECUTABLE
		NAMES ${RUBY_EXECUTABLE_NAMES}
		PATHS /usr/bin /usr/local/bin /usr/pkg/bin
	)

	if(RUBY_EXECUTABLE)
		execute_process(
			COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['rubyhdrdir'] || RbConfig::CONFIG['archdir']"
			OUTPUT_VARIABLE RUBY_ARCH_DIR
		)

		execute_process(
			COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['arch']"
			OUTPUT_VARIABLE RUBY_ARCH
		)

		execute_process(
			COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['libdir']"
			OUTPUT_VARIABLE RUBY_POSSIBLE_LIB_PATH
		)

		execute_process(
			COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['rubylibdir']"
			OUTPUT_VARIABLE RUBY_RUBY_LIB_PATH
		)

		execute_process(
			COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['MAJOR']"
			OUTPUT_VARIABLE RUBY_VERSION_MAJOR
		)

		execute_process(
			COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['MINOR']"
			OUTPUT_VARIABLE RUBY_VERSION_MINOR
		)

		execute_process(
			COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['TEENY']"
			OUTPUT_VARIABLE RUBY_VERSION_PATCH
		)

		set(RUBY_VERSION "${RUBY_VERSION_MAJOR}.${RUBY_VERSION_MINOR}.${RUBY_VERSION_PATCH}")

		find_path(RUBY_INCLUDE_DIRS
			NAMES ruby.h
			PATHS ${RUBY_ARCH_DIR}
		)

		set(RUBY_INCLUDE_ARCH "${RUBY_INCLUDE_DIRS}/${RUBY_ARCH}")

		set(RUBY_LIBRARY_NAMES
			ruby-1.9.3 ruby1.9.3 ruby193 ruby-1.9.2 ruby1.9.2 ruby192
			ruby-1.9.1 ruby1.9.1 ruby191 ruby1.9 ruby19 ruby1.8 ruby18 ruby
		)

		set(RUBY_LIBRARY_DIRS
			${RUBY_POSSIBLE_LIB_PATH}
			${RUBY_RUBY_LIB_PATH}
		)

		find_library(RUBY_LIBRARY
			NAMES ${RUBY_LIBRARY_NAMES}
			PATHS ${RUBY_LIBRARY_DIRS}
		)

		if(RUBY_LIBRARY AND RUBY_INCLUDE_DIRS)
			set(RUBY_FOUND TRUE)
		endif()

		set(RUBY_INCLUDE_DIRS "${RUBY_INCLUDE_DIRS};${RUBY_INCLUDE_ARCH}")

	endif()
endif()

mark_as_advanced(
	RUBY_INCLUDE_DIRS
	RUBY_LIBRARY_DIRS
	RUBY_LIBRARY
	RUBY_VERSION_MAJOR
	RUBY_VERSION_MINOR
	RUBY_VERSION_PATCH
	RUBY_VERSION
)

if(_RUBY_CMAKE_DEBUG)
	message(STATUS "------------ FindRubyEx.cmake Debug ------------")
	message(STATUS "RUBY_INCLUDE_DIRS = ${RUBY_INCLUDE_DIRS}")
	message(STATUS "RUBY_LIBRARY_DIRS = ${RUBY_LIBRARY_DIRS}")
	message(STATUS "RUBY_LIBRARY = ${RUBY_LIBRARY}")
	message(STATUS "RUBY_VERSION_MAJOR = ${RUBY_VERSION_MAJOR}")
	message(STATUS "RUBY_VERSION_MINOR = ${RUBY_VERSION_MINOR}")
	message(STATUS "RUBY_VERSION_PATCH = ${RUBY_VERSION_PATCH}")
	message(STATUS "RUBY_VERSION = ${RUBY_VERSION}")
	message(STATUS "------------ FindRubyEx.cmake Debug ------------")
endif()
