
# Check if scripts are enabled
if(NOT OPTION_BUILD_SCRIPTS)
    return()
endif()

# Script directory packages
add_subdirectory(c)
add_subdirectory(javascript)
add_subdirectory(python)
add_subdirectory(ruby)
add_subdirectory(csharp)
