
# Check if ports are enabled
if(NOT OPTION_BUILD_PORTS)
    return()
endif()

# Port interface
add_subdirectory(interface)

# Port languages
add_subdirectory(csharp)
add_subdirectory(d)
add_subdirectory(go)
add_subdirectory(java)
add_subdirectory(javascript)
add_subdirectory(lua)
add_subdirectory(perl)
add_subdirectory(php)
add_subdirectory(python)
add_subdirectory(ruby)
