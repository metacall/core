#
# Configure ruby project
#

rb_project(blog 0.1.0)

configure_file("data/blog.rb.in" "${LOADER_SCRIPT_PATH}/blog.rb" @ONLY)

set(TIMESTAMP_PATH "${LOADER_SCRIPT_PATH}/blog_test_bundled")

if(NOT EXISTS "${TIMESTAMP_PATH}")
	execute_process(
		COMMAND bundle install
		WORKING_DIRECTORY ${LOADER_SCRIPT_PATH}
	)
endif()

execute_process(
	COMMAND ${CMAKE_COMMAND} -E touch ${TIMESTAMP_PATH}
)
