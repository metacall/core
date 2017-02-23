#
# Configure ruby project
#

rb_project(blog 0.1.0)

configure_file("data/blog.rb.in" "${LOADER_SCRIPT_PATH}/blog.rb" @ONLY)
