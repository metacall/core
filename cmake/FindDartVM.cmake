#
# CMake Find Dart Runtime by Parra Studios Copyright (C) 2016 - 2021 Vicente
# Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Attempts to find the paths to Dart SDK include files and libraries, based on
# the path of the `dart` executable.
#
# Use as follows:
#
# find_package(DartVM REQUIRED)
#
# The following variables will be set:
#
# DARTVM_FOUND - TRUE if Dart was found DARTVM_ROOT_DIR - The Dart SDK root
# DARTVM_EXECUTABLE - The `dart` executable DARTVM_INCLUDE_DIR - The `include`
# directory within the SDK DARTVM_LIBRARY - The name of the `libdart` library
#
# The following macro is added as well:
#
# add_DARTVM_native_extension(NAME srcs...) This is a very convenient macro that
# automatically performs all necessary configuration to build a Dart native
# extension.

find_program(_DARTVM_EXECUTABLE dart)

if("${_DARTVM_EXECUTABLE}" STREQUAL "_DARTVM_EXECUTABLE-NOTFOUND")
  set(DARTVM_FOUND FALSE)
else()
  get_filename_component(DARTVM_EXECUTABLE ${_DARTVM_EXECUTABLE} REALPATH)
  set(DARTVM_FOUND TRUE)
  mark_as_advanced(DARTVM_EXECUTABLE)
endif()

if(DARTVM_FOUND)
  get_filename_component(_DARTVM_BIN ${DARTVM_EXECUTABLE} DIRECTORY)
  get_filename_component(DARTVM_ROOT_DIR ${_DARTVM_BIN} DIRECTORY)
  mark_as_advanced(DARTVM_ROOT_DIR)

  if(WIN32)
    set(DARTVM_INCLUDE_DIR "${DARTVM_ROOT_DIR}\include")
    set(DARTVM_LIBRARY "${_DARTVM_BIN}\dart.lib")
    mark_as_advanced(DARTVM_INCLUDE_DIR DARTVM_LIBRARY)
  else()
    set(DARTVM_INCLUDE_DIR "${DARTVM_ROOT_DIR}/include")
    mark_as_advanced(DARTVM_INCLUDE_DIR)
  endif()
endif()

macro(add_DARTVM_native_extension NAME)
  add_library(${NAME} SHARED ${ARGN})
  target_include_directories(${NAME} PUBLIC ${DARTVM_INCLUDE_DIR})
  target_compile_definitions(${NAME} PUBLIC DARTVM_SHARED_LIB=1)

  # if(APPLE) target_link_libraries(${NAME} "-undefined dynamic_lookup") endif()

  if(WIN32)
    target_link_libraries(${NAME} ${DARTVM_LIBRARY})
  else()
    set_target_properties(${NAME} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
  endif()
endmacro()
