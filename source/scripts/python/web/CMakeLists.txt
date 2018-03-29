#
# Configure python project
#

py_project(web 0.1.0)

# Configure index path
set(PROJECT_METACALL_INDEX_PATH "${LOADER_SCRIPT_PATH}/index.html")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/source/web.py.in" "${LOADER_SCRIPT_PATH}/web.py" @ONLY)
