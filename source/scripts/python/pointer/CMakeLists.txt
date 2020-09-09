#
# Configure python project
#

py_project(pointer 0.1.0)

# Configure ports path
set(PROJECT_METACALL_PORTS_DIRECTORY "${CMAKE_SOURCE_DIR}/source/ports/py_port")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/source/pointer.py.in" "${LOADER_SCRIPT_PATH}/pointer.py" @ONLY)
