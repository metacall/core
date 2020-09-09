#
# Configure python project
#

py_project(landing 0.1.0)

# Configure port path
set(PROJECT_METACALL_PORTS_DIRECTORY "${CMAKE_SOURCE_DIR}/source/ports/py_port")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/source/landing.py.in" "${LOADER_SCRIPT_PATH}/landing.py" @ONLY)
