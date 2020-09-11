# Configure Python Port path
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test/cli-test.py.in" "${LOADER_SCRIPT_PATH}/cli-test.py" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test/cli-test-rb.py.in" "${LOADER_SCRIPT_PATH}/cli-test-rb.py" @ONLY)
