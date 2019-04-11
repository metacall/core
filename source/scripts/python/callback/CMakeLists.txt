#
# Configure python project
#

py_project(callback 0.1.0)

# TODO: Review this, set ports as a dependency in requirements.txt

# Configure ports path
if(MSVC)
	set(PROJECT_METACALL_PORTS_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
	set(PROJECT_METACALL_PORTS_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(PROJECT_METACALL_PORTS_PY "_py_portd")
else()
	set(PROJECT_METACALL_PORTS_PY "_py_port")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/source/callback_metacall.py.in" "${LOADER_SCRIPT_PATH}/callback_metacall.py" @ONLY)
