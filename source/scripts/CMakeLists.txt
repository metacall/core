# Check if scripts are enabled
if(NOT OPTION_BUILD_SCRIPTS)
	return()
endif()

if(NOT LOADER_SCRIPT_PATH)
	return()
endif()

# Create output directory
file(MAKE_DIRECTORY ${LOADER_SCRIPT_PATH})

# Plugins options
option(OPTION_BUILD_SCRIPTS_C "Build C scripts." ON)
option(OPTION_BUILD_SCRIPTS_CS "Build C# scripts." ON)
option(OPTION_BUILD_SCRIPTS_JS "Build Java scripts." ON)
option(OPTION_BUILD_SCRIPTS_JS "Build JavaScript scripts." ON)
option(OPTION_BUILD_SCRIPTS_PY "Build Python scripts." ON)
option(OPTION_BUILD_SCRIPTS_RB "Build Ruby scripts." ON)

# Script directory packages
add_subdirectory(c)
add_subdirectory(csharp)
add_subdirectory(java)
add_subdirectory(javascript)
add_subdirectory(python)
add_subdirectory(ruby)
