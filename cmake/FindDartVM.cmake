#
# CMake Find Dart Runtime by Parra Studios Copyright (C) 2016 - 2021 Vicente
# Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Attempts to find the paths to Dart SDK include files and libraries, based on
# the path of the `dart` executable.
#
# Use as follows:
#
# find_package(DART REQUIRED)
#
# The following variables will be set:
#
# DART_FOUND - TRUE if Dart was found DART_ROOT_DIR - The Dart SDK root
# DART_EXECUTABLE - The `dart` executable DART_INCLUDE_DIR - The `include`
# directory within the SDK DART_LIBRARY - The name of the `libdart` library
#
# The following macro is added as well:
#
# add_DART_native_extension(NAME srcs...) This is a very convenient macro that
# automatically performs all necessary configuration to build a Dart native
# extension.

find_program(_DART_EXECUTABLE dart)

if("${_DART_EXECUTABLE}" STREQUAL "_DART_EXECUTABLE-NOTFOUND")
  set(DART_FOUND FALSE)
else()
  get_filename_component(DART_EXECUTABLE ${_DART_EXECUTABLE} REALPATH)
  set(DART_FOUND TRUE)
  mark_as_advanced(DART_EXECUTABLE)
endif()

if(DART_FOUND)
  get_filename_component(_DART_BIN ${DART_EXECUTABLE} DIRECTORY)
  get_filename_component(DART_ROOT_DIR ${_DART_BIN} DIRECTORY)
  mark_as_advanced(DART_ROOT_DIR)

  if(WIN32)
    set(DART_INCLUDE_DIR "${DART_ROOT_DIR}\include")
    set(DART_LIBRARY "${_DART_BIN}\dart.lib")
    mark_as_advanced(DART_INCLUDE_DIR DART_LIBRARY)
  else()
    set(DART_INCLUDE_DIR "${DART_ROOT_DIR}/include")
    mark_as_advanced(DART_INCLUDE_DIR)
  endif()
endif()

macro(add_DART_native_extension NAME)
  add_library(${NAME} SHARED ${ARGN})
  target_include_directories(${NAME} PUBLIC ${DART_INCLUDE_DIR})
  target_compile_definitions(${NAME} PUBLIC DART_SHARED_LIB=1)

  # if(APPLE) target_link_libraries(${NAME} "-undefined dynamic_lookup") endif()

  if(WIN32)
    target_link_libraries(${NAME} ${DART_LIBRARY})
  else()
    set_target_properties(${NAME} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
  endif()
endmacro()
