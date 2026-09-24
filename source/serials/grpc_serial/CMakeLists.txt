# Check if this serial is enabled
if(NOT OPTION_BUILD_SERIALS OR NOT OPTION_BUILD_SERIALS_GRPC)
	return()
endif()

#
# External dependencies
#

find_package(GRPC)

if(NOT GRPC_FOUND)
	include(InstallGRPC)

	if(NOT GRPC_FOUND)
		message(SEND_ERROR "gRPC libraries not found")
		return()
	endif()

	set(GRPC_INSTALL TRUE)
endif()

#
# Library name and options
#

set(target grpc_serial)

message(STATUS "Serial ${target}")

string(TOUPPER ${target} target_upper)
set(export_file  "include/${target}/${target}_api.h")
set(export_macro "${target_upper}_API")

include(Warnings)
include(SecurityFlags)

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(headers
	${include_path}/grpc_serial.h
	${include_path}/grpc_serial_impl.h
)

set(sources
	${source_path}/grpc_serial.c
	${source_path}/grpc_serial_impl.cpp
)

set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$"
	${header_group} ${headers})
source_group_by_path(${source_path}  "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$"
	${source_group} ${sources})

add_library(${target} MODULE
	${sources}
	${headers}
)

add_library(${META_PROJECT_NAME}::${target} ALIAS ${target})

export(TARGETS ${target} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

generate_export_header(${target}
	EXPORT_FILE_NAME  ${export_file}
	EXPORT_MACRO_NAME ${export_macro}
)

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
	BUNDLE $<AND:$<PLATFORM_ID:Darwin>,$<VERSION_GREATER:${PROJECT_OS_VERSION},8>>
)

target_include_directories(${target}
	PRIVATE
	${PROJECT_BINARY_DIR}/source/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::metacall,INCLUDE_DIRECTORIES>
	${GRPC_INCLUDE_DIRS}
	PUBLIC
	${DEFAULT_INCLUDE_DIRECTORIES}
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

target_link_libraries(${target}
	PRIVATE
	$<$<NOT:$<PLATFORM_ID:Darwin>>:${META_PROJECT_NAME}::metacall>
	${GRPC_LIBRARIES}
	PUBLIC
	${DEFAULT_LIBRARIES}
	INTERFACE
)

target_compile_definitions(${target}
	PRIVATE
	PUBLIC
	$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target_upper}_STATIC_DEFINE>
	${DEFAULT_COMPILE_DEFINITIONS}
	INTERFACE
)

target_compile_options(${target}
	PRIVATE
	PUBLIC
	${DEFAULT_COMPILE_OPTIONS}
	INTERFACE
)

target_link_options(${target}
	PRIVATE
	$<$<AND:$<PLATFORM_ID:Darwin>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-Wl,-undefined,dynamic_lookup>
	PUBLIC
	${DEFAULT_LINKER_OPTIONS}
	INTERFACE
)

install(TARGETS ${target}
	EXPORT  "${target}-export"              COMPONENT dev
	RUNTIME DESTINATION ${INSTALL_BIN}      COMPONENT runtime
	LIBRARY DESTINATION ${INSTALL_SHARED}   COMPONENT runtime
	ARCHIVE DESTINATION ${INSTALL_LIB}      COMPONENT dev
)