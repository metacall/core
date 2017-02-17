#
# Configure python project
#

py_project(frontend 0.1.0)

#
# External dependencies
#

set(Python_ADDITIONAL_VERSIONS 3.4)

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

set(target python-frontend-migrate)

add_custom_target(${target} ALL
	COMMAND ${PYTHON_EXECUTABLE} ${LOADER_SCRIPT_PATH}/manage.py migrate
)

set_target_properties(${target}
	PROPERTIES
	FOLDER "${IDE_FOLDER}/Python"
)
