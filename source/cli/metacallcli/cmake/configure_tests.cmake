# Configure Python Port path
set(PROJECT_METACALL_PORTS_DIRECTORY "${CMAKE_SOURCE_DIR}/source/ports/py_port")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test/cli-test.py.in" "${LOADER_SCRIPT_PATH}/cli-test.py" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test/cli-test-rb.py.in" "${LOADER_SCRIPT_PATH}/cli-test-rb.py" @ONLY)
