#
# Configure python project
#

py_project(frontend 0.1.0)

#
# External dependencies
#

set(Python_ADDITIONAL_VERSIONS 3.5)

find_package(PythonInterp REQUIRED)

if(NOT PYTHONINTERP_FOUND)
	message(STATUS "Python interpreter not found")
	return()
endif()

# Build frontend
if(NOT ${PYTHON_VERSION_MAJOR} EQUAL 3)
	message(WARNING "Invalid Python executable version: ${PYTHON_VERSION_STRING}")
	message(WARNING "Define a valid Python 3 executable in variable PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE} or ${target} test won't be compiled")
	message(WARNING "Use: \"sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10\" to modify default python executable")
	return()
endif()

# TODO

# set(target python-frontend-migrate)

# add_custom_target(${target} ALL
# 	COMMAND ${PYTHON_EXECUTABLE} -m pip install --upgrade pip
# 	COMMAND ${PYTHON_EXECUTABLE} -m pip install --install-option="--prefix=${CMAKE_CURRENT_BINARY_DIR}" -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt
# 	COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR};$ENV{PYTHONPATH}" ${PYTHON_EXECUTABLE} ${LOADER_SCRIPT_PATH}/manage.py migrate
# )

# set_target_properties(${target}
# 	PROPERTIES
# 	FOLDER "${IDE_FOLDER}/Python"
# )
