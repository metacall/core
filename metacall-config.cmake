
# This config script tries to locate the project either in its source tree
# of from an install location.
# 
# Please adjust the list of submodules to search for.


# List of modules
set(MODULE_NAMES
    baselib
    fiblib
)


# Macro to search for a specific module
macro(find_module FILENAME)
    if(EXISTS "${FILENAME}")
        set(MODULE_FOUND TRUE)
        include("${FILENAME}")
    endif()
endmacro()

# Macro to search for all modules
macro(find_modules PREFIX)
    foreach(module_name ${MODULE_NAMES})
        find_module("${CMAKE_CURRENT_LIST_DIR}/${PREFIX}/${module_name}/${module_name}-export.cmake")
    endforeach(module_name)
endmacro()


# Try install location
set(MODULE_FOUND FALSE)
find_modules("cmake")

if(MODULE_FOUND)
    return()
endif()

# Try common build locations
find_modules("build/cmake")
find_modules("build-debug/cmake")
