# Check if detours are enabled
if(NOT OPTION_BUILD_DETOURS)
	return()
endif()

# Detour options
option(OPTION_BUILD_DETOURS_PLTHOOK "PLTHook library detour." ON)

# Detour packages
add_subdirectory(plthook_detour) # PLTHook library
