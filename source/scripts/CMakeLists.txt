
# Check if scripts are enabled
if(NOT OPTION_BUILD_SCRIPTS)
    return()
endif()

# Plugin packages
add_subdirectory(c)
add_subdirectory(javascript)
add_subdirectory(python)
add_subdirectory(ruby)
