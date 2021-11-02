set(GOPATH "${CMAKE_CURRENT_BINARY_DIR}/go")
file(MAKE_DIRECTORY ${GOPATH})

function(ExternalGoProject_Add TARG)
	add_custom_target(${TARG} env GOPATH=${GOPATH} ${CMAKE_Go_COMPILER} get ${ARGN})
endfunction(ExternalGoProject_Add)

function(add_go_executable NAME)
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/.timestamp 
		COMMAND env GOPATH=${GOPATH} ${CMAKE_Go_COMPILER} build
		-o "${PROJECT_OUTPUT_DIR}/${NAME}"
		${CMAKE_GO_FLAGS} ${ARGN}
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

	add_custom_target(${NAME} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/.timestamp ${ARGN})
	install(PROGRAMS ${PROJECT_OUTPUT_DIR}/${NAME} DESTINATION bin)
endfunction(add_go_executable)

function(add_go_library NAME BUILD_TYPE)
	if(BUILD_TYPE STREQUAL "STATIC")
		set(BUILD_MODE -buildmode=c-archive)
		set(LIB_NAME "lib${NAME}.a")
	else()
		set(BUILD_MODE -buildmode=c-shared)
		if(APPLE)
			set(LIB_NAME "lib${NAME}.dylib")
		else()
			set(LIB_NAME "lib${NAME}.so")
		endif()
	endif()

	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/.timestamp
		COMMAND env GOPATH=${GOPATH} ${CMAKE_Go_COMPILER} build ${BUILD_MODE}
		-o "${PROJECT_OUTPUT_DIR}/${LIB_NAME}"
		${CMAKE_GO_FLAGS} ${ARGN}
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

	add_custom_target(${NAME} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/.timestamp ${ARGN})

	if(NOT BUILD_TYPE STREQUAL "STATIC")
		install(PROGRAMS ${PROJECT_OUTPUT_DIR}/${LIB_NAME} DESTINATION bin)
	endif()
endfunction(add_go_library)
