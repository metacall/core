#
# Configure python project
#

py_project(host 0.1.0)

# Configure port path
set(PROJECT_METACALL_PORTS_DIRECTORY "${CMAKE_SOURCE_DIR}/source/ports/py_port")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/source/host.py.in" "${LOADER_SCRIPT_PATH}/host.py" @ONLY)
