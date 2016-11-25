#
# External dependencies
#

find_package(PythonLibs)

if(NOT PYTHONLIBS_FOUND)
    message(STATUS "Python libraries not found")
    return()
endif()

#
# Port name and options
#

# Target name
set(target py_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

# Set API export file and macro
string(TOUPPER ${target} target_upper)
set(feature_file "include/${target}/${target}_features.h")
set(export_file  "include/${target}/${target}_api.h")
set(export_macro "${target_upper}_API")

#
# Sources
#

set(metacall_interface_path "${CMAKE_SOURCE_DIR}/source/ports/interface/metacall")
set(interface_path "${CMAKE_CURRENT_SOURCE_DIR}/interface/${target}")
set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(interfaces
    ${metacall_interface_path}/metacall.i

    ${interface_path}/py_port.i
    ${interface_path}/py_port_typemap.i
)

set(headers
    ${include_path}/py_port.h
)

set(sources
    ${source_path}/py_port.c
)

# Group source files
set(interface_group "Interface Files (SWIG)")
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${interface_path} "\\\\.i$"
    ${interface_group} ${interfaces})
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$"
    ${header_group} ${headers})
source_group_by_path(${source_path}  "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$"
    ${source_group} ${sources})

#
# SWIG Configuration
#

# Set SWIG flags
set(CMAKE_SWIG_FLAGS "")

# Set SWIG include path
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

#
# Create library
#

foreach(files ${interfaces} ${headers} ${sources})
    set_source_files_properties(
        ${interfaces}
        PROPERTY SWIG_FLAGS "-python" "-includeall"
    )

    set_source_files_properties(
        ${interfaces}
        PROPERTIES CPLUSPLUS OFF
    )
endforeach()

# Build library
macro(swig_add_depends target)
    foreach(dependency ${ARGN})
        get_property(include_list
            TARGET ${dependency}
            PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        )

        foreach(path ${include_list})
            include_directories(${path})

            file(GLOB_RECURSE header_files "${path}/*.[h|hpp]")

            list(APPEND SWIG_MODULE_${target}_EXTRA_DEPS ${header_files})

            set(SWIG_MODULE_${target}_EXTRA_DEPS SWIG_MODULE_${target}_EXTRA_DEPS PARENT_SCOPE)

        endforeach()
    endforeach()
endmacro()

swig_add_depends(${target}
    ${META_PROJECT_NAME}::preprocessor
    ${META_PROJECT_NAME}::log
    ${META_PROJECT_NAME}::adt
    ${META_PROJECT_NAME}::dynlink
    ${META_PROJECT_NAME}::reflect
    ${META_PROJECT_NAME}::loader
    ${META_PROJECT_NAME}::metacall
)

swig_add_module(${target}
    python
    ${interfaces}
    ${headers}
    ${sources}
)

# Create namespaced alias
add_library(${META_PROJECT_NAME}::${target} ALIAS ${SWIG_MODULE_${target}_REAL_NAME})

# Export library for downstream projects
export(TARGETS ${SWIG_MODULE_${target}_REAL_NAME} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

# Create feature detection header
# Compilers: https://cmake.org/cmake/help/v3.1/variable/CMAKE_LANG_COMPILER_ID.html#variable:CMAKE_%3CLANG%3E_COMPILER_ID
# Feature: https://cmake.org/cmake/help/v3.1/prop_gbl/CMAKE_CXX_KNOWN_FEATURES.html

# Check for availability of module; use pre-generated version if not found
if (WriterCompilerDetectionHeaderFound)
    write_compiler_detection_header(
        FILE ${feature_file}
        PREFIX ${target_upper}
        COMPILERS AppleClang Clang GNU MSVC
        FEATURES cxx_alignas cxx_alignof cxx_constexpr cxx_final cxx_noexcept cxx_nullptr cxx_sizeof_member cxx_thread_local
        VERSION 3.2
    )
else()
    file(
        COPY ${PROJECT_SOURCE_DIR}/codegeneration/${target}_features.h
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/${target}
        USE_SOURCE_PERMISSIONS
    )
endif()

# Create API export header
generate_export_header(${SWIG_MODULE_${target}_REAL_NAME}
    EXPORT_FILE_NAME  ${export_file}
    EXPORT_MACRO_NAME ${export_macro}
)

#
# Project options
#

set_target_properties(${SWIG_MODULE_${target}_REAL_NAME}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    FOLDER "${IDE_FOLDER}"
)

#
# Include directories
#
target_include_directories(${SWIG_MODULE_${target}_REAL_NAME}
    PRIVATE
    ${PROJECT_BINARY_DIR}/source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include

    ${PYTHON_INCLUDE_DIRS} # Python includes

    PUBLIC
    ${DEFAULT_INCLUDE_DIRECTORIES}

    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

#
# Libraries
#

swig_link_libraries(${target}
    PRIVATE
    ${META_PROJECT_NAME}::preprocessor
    ${META_PROJECT_NAME}::log
    ${META_PROJECT_NAME}::adt
    ${META_PROJECT_NAME}::dynlink
    ${META_PROJECT_NAME}::reflect
    ${META_PROJECT_NAME}::loader
    ${META_PROJECT_NAME}::metacall

    ${PYTHON_LIBRARIES} # Python libraries

    PUBLIC
    ${DEFAULT_LIBRARIES}

    INTERFACE
)

#
# Compile definitions
#

target_compile_definitions(${SWIG_MODULE_${target}_REAL_NAME}
    PRIVATE

    PUBLIC
    $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target_upper}_STATIC_DEFINE>
    ${DEFAULT_COMPILE_DEFINITIONS}

    INTERFACE
)

#
# Compile options
#

target_compile_options(${SWIG_MODULE_${target}_REAL_NAME}
    PRIVATE

    PUBLIC
    ${DEFAULT_COMPILE_OPTIONS}

    INTERFACE
)

#
# Linker options
#

target_link_libraries(${SWIG_MODULE_${target}_REAL_NAME}
    PRIVATE

    PUBLIC
    ${DEFAULT_LINKER_OPTIONS}

    INTERFACE
)

#
# Deployment
#

# Library
install(TARGETS ${SWIG_MODULE_${target}_REAL_NAME}
    EXPORT  "${target}-export"            COMPONENT dev
    RUNTIME DESTINATION ${INSTALL_BIN}    COMPONENT runtime
    LIBRARY DESTINATION ${INSTALL_SHARED} COMPONENT runtime
    ARCHIVE DESTINATION ${INSTALL_LIB}    COMPONENT dev
)

# Header files
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE}
    COMPONENT dev
)

# Generated header files
install(DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE}
    COMPONENT dev
)

# CMake config
install(EXPORT ${target}-export
    NAMESPACE   ${META_PROJECT_NAME}::
    DESTINATION ${INSTALL_CMAKE}/${target}
    COMPONENT   dev
)
