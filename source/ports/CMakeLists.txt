# Check if ports are enabled
if(NOT OPTION_BUILD_PORTS)
    return()
endif()

# Port interface
add_subdirectory(interface)

# Port languages
add_subdirectory(cs_port)
add_subdirectory(d_port)
add_subdirectory(go_port)
add_subdirectory(java_port)
#add_subdirectory(js_port)
add_subdirectory(lua_port)
#add_subdirectory(node_port)
add_subdirectory(php_port)
add_subdirectory(pl_port)
add_subdirectory(py_port)
add_subdirectory(rb_port)
add_subdirectory(r_port)
