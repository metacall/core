set(GOPATH "${CMAKE_CURRENT_BINARY_DIR}/go")
file(MAKE_DIRECTORY ${GOPATH})

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMakeDetermineGoCompiler.cmake)

function(ExternalGoProject_Add TARG)
	add_custom_target(${TARG} ${CMAKE_COMMAND} -E env GOPATH=${GOPATH} ${CMAKE_Go_COMPILER} get ${ARGN})
endfunction(ExternalGoProject_Add)

function(add_go_executable NAME)
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/.timestamp 
		COMMAND ${CMAKE_COMMAND} -E env GOPATH=${GOPATH} ${CMAKE_CGO_FLAGS} ${CMAKE_Go_COMPILER} build
		-o "${PROJECT_OUTPUT_DIR}/${NAME}"
		${CMAKE_GO_FLAGS} ${ARGN}
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

	add_custom_target(${NAME} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/.timestamp ${ARGN})
	install(PROGRAMS ${PROJECT_OUTPUT_DIR}/${NAME} DESTINATION bin)
endfunction(add_go_executable)

function(add_go_library NAME BUILD_TYPE)
	if(BUILD_TYPE STREQUAL "STATIC")
		if(WIN32)
			set(LIB_NAME "lib${NAME}.lib")
		else()
			set(LIB_NAME "lib${NAME}.a")
		endif()
	else()
		if(APPLE)
			set(LIB_NAME "lib${NAME}.dylib")
		elseif(WIN32)
			set(LIB_NAME "lib${NAME}.dll")
		else()
			set(LIB_NAME "lib${NAME}.so")
		endif()
	endif()

	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/.timestamp
		COMMAND ${CMAKE_COMMAND} -E env GOPATH=${GOPATH} ${CMAKE_CGO_FLAGS} ${CMAKE_Go_COMPILER} build ${BUILD_MODE}
		-o "${PROJECT_OUTPUT_DIR}/${LIB_NAME}"
		${CMAKE_GO_FLAGS} ${ARGN}
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

	add_custom_target(${NAME} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/.timestamp ${ARGN})

	if(NOT BUILD_TYPE STREQUAL "STATIC")
		install(PROGRAMS ${PROJECT_OUTPUT_DIR}/${LIB_NAME} DESTINATION bin)
	endif()
endfunction(add_go_library)
