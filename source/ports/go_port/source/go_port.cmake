# Sanitize the variables
string(REPLACE "\"" "" GOPATH "${GOPATH}")
string(REPLACE "\"" "" CGO_CFLAGS "${CGO_CFLAGS}")
string(REPLACE "\\" "" CGO_CFLAGS "${CGO_CFLAGS}")
string(REPLACE "\"" "" CGO_LDFLAGS "${CGO_LDFLAGS}")
string(REPLACE "\\" "" CGO_LDFLAGS "${CGO_LDFLAGS}")
string(REPLACE "\"" "" CMAKE_Go_COMPILER "${CMAKE_Go_COMPILER}")
string(REPLACE "\"" "" LIBRARY_PATH_NAME "${LIBRARY_PATH_NAME}")
string(REPLACE "\"" "" LIBRARY_PATH "${LIBRARY_PATH}")
string(REPLACE "\"" "" COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS}")
string(REPLACE "\"" "" GO_TASK "${GO_TASK}")
string(REPLACE "\"" "" GO_FLAGS "${GO_FLAGS}")
string(REPLACE "\"" "" GO_CC "${GO_CC}")
string(REPLACE "\"" "" GO_SANITIZER_CFLAGS "${GO_SANITIZER_CFLAGS}")
string(REPLACE "\"" "" GO_SANITIZER_LDFLAGS "${GO_SANITIZER_LDFLAGS}")
string(REPLACE "\"" "" GO_OUTPUT_FILE "${GO_OUTPUT_FILE}")

# Memory check and sanitizer definitions (__MEMORYCHECK__, __ADDRESS_SANITIZER__...)
separate_arguments(COMPILE_DEFINITIONS UNIX_COMMAND "${COMPILE_DEFINITIONS}")

foreach(DEFINITION ${COMPILE_DEFINITIONS})
	set(CGO_CFLAGS "${CGO_CFLAGS} -D${DEFINITION}")
endforeach()

# Go flags (-asan, -msan or -race)
separate_arguments(GO_FLAGS UNIX_COMMAND "${GO_FLAGS}")

# Use the same C compiler as metacall when sanitizers are enabled
if(NOT "${GO_CC}" STREQUAL "")
	set(GO_ENV CC=${GO_CC})
endif()

# Sanitizer flags of metacall, passed by environment because the cgo allowlist only applies to #cgo directives
# and it rejects most of them (i.e -fno-optimize-sibling-calls), the CGO_CFLAGS default (-g -O2) is kept
if(NOT "${GO_SANITIZER_CFLAGS}" STREQUAL "")
	list(APPEND GO_ENV "CGO_CFLAGS=-g -O2 ${GO_SANITIZER_CFLAGS}")
endif()

if(NOT "${GO_SANITIZER_LDFLAGS}" STREQUAL "")
	list(APPEND GO_ENV "CGO_LDFLAGS=${GO_SANITIZER_LDFLAGS}")
endif()

# Override CGO flags in all files that contain them
file(GLOB GO_FILES "${CMAKE_CURRENT_LIST_DIR}/*.go")
set(CGO_FILES)

foreach(GO_FILE ${GO_FILES})
	file(READ "${GO_FILE}" FILE_CONTENTS)
	string(FIND "${FILE_CONTENTS}" "#cgo CFLAGS: -Wall" CGO_CFLAGS_POS)
	string(FIND "${FILE_CONTENTS}" "#cgo LDFLAGS: -lmetacall" CGO_LDFLAGS_POS)
	if(CGO_CFLAGS_POS EQUAL -1 AND CGO_LDFLAGS_POS EQUAL -1)
		continue()
	endif()
	list(APPEND CGO_FILES "${GO_FILE}")
	string(REPLACE "#cgo CFLAGS: -Wall" "#cgo CFLAGS: ${CGO_CFLAGS}" FILE_CONTENTS "${FILE_CONTENTS}")
	string(REPLACE "#cgo LDFLAGS: -lmetacall" "#cgo LDFLAGS: ${CGO_LDFLAGS}" FILE_CONTENTS "${FILE_CONTENTS}")
	file(WRITE "${GO_FILE}" "${FILE_CONTENTS}")
endforeach()

execute_process(COMMAND
	${CMAKE_COMMAND} -E env ${GO_ENV} GOPATH=${GOPATH} ${LIBRARY_PATH_NAME}=${LIBRARY_PATH} ${CMAKE_Go_COMPILER} ${GO_TASK} ${GO_FLAGS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
	RESULT_VARIABLE RESULT
    OUTPUT_VARIABLE OUTPUT
    ERROR_VARIABLE ERROR
)

# Restore CGO flags
foreach(GO_FILE ${CGO_FILES})
	file(READ "${GO_FILE}" FILE_CONTENTS)
	string(REPLACE "#cgo CFLAGS: ${CGO_CFLAGS}" "#cgo CFLAGS: -Wall" FILE_CONTENTS "${FILE_CONTENTS}")
	string(REPLACE "#cgo LDFLAGS: ${CGO_LDFLAGS}" "#cgo LDFLAGS: -lmetacall" FILE_CONTENTS "${FILE_CONTENTS}")
	file(WRITE "${GO_FILE}" "${FILE_CONTENTS}")
endforeach()

message(STATUS "${OUTPUT}")

# Store the output (used for benchmarks)
if(NOT "${GO_OUTPUT_FILE}" STREQUAL "")
	file(WRITE "${GO_OUTPUT_FILE}" "${OUTPUT}")
endif()

if(RESULT)
	set(GO_PORT_ERROR "Go port failed with result: ${RESULT}\n${ERROR}")

	# When included from another script (i.e profiler), the error is deferred so the caller can handle it
	if(NOT GO_DEFER_ERROR)
		message(FATAL_ERROR "${GO_PORT_ERROR}")
	endif()
endif()
