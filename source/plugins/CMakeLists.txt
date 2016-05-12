
# Check if plugins are enabled
if(NOT OPTION_BUILD_PLUGINS)
    return()
endif()

# Plugin packages
#add_subdirectory(js_loader)
add_subdirectory(py_loader)
#add_subdirectory(rb_loader)
