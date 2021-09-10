#
# External dependencies
#

set(Python_ADDITIONAL_VERSIONS 3)

find_package(PythonInterp REQUIRED)

if(NOT PYTHONINTERP_FOUND)
	message(STATUS "Python interpreter not found")
	return()
endif()

if(NOT ${PYTHON_VERSION_MAJOR} EQUAL 3)
	message(WARNING "Invalid Python executable version: ${PYTHON_VERSION_STRING}")
	message(WARNING "Define a valid Python 3 executable in variable PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE} or ${target} test won't be compiled")
	message(WARNING "Use: \"sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10\" to modify default python executable")
	return()
endif()

# Detect if Django is available
execute_process(
	COMMAND ${PYTHON_EXECUTABLE} -c "import django"
	RESULT_VARIABLE PYTHON_DJANGO_FOUND
	ERROR_QUIET
	OUTPUT_QUIET
)

if("${PYTHON_DJANGO_FOUND}" EQUAL "1")
	message(WARNING "Python Django not found, skipping the python frontend script project")
	return()
endif()

#
# Configure python project
#

py_project(frontend 0.1.0)

set(target python-frontend-migrate)

# Build frontend
add_custom_target(${target} ALL
	COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH="${CMAKE_CURRENT_BINARY_DIR};$ENV{PYTHONPATH}" ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/source/manage.py migrate
	WORKING_DIRECTORY ${LOADER_SCRIPT_PATH}
	DEPENDS python-frontend
)

set_target_properties(${target}
	PROPERTIES
	FOLDER "${IDE_FOLDER}/Python"
)
