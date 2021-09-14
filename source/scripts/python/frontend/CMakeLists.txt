#
# External dependencies
#

find_package(Python3 COMPONENTS Interpreter REQUIRED)

if(NOT Python3_Interpreter_FOUND)
	message(STATUS "Python interpreter not found")
	return()
endif()

# Detect if Django is available
execute_process(
	COMMAND ${Python_EXECUTABLE} -c "import django"
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
	COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH="${CMAKE_CURRENT_BINARY_DIR};$ENV{PYTHONPATH}" ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/source/manage.py migrate
	WORKING_DIRECTORY ${LOADER_SCRIPT_PATH}
	DEPENDS python-frontend
)

set_target_properties(${target}
	PROPERTIES
	FOLDER "${IDE_FOLDER}/Python"
)
