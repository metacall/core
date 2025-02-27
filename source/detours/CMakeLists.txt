# Check if detours are enabled
if(NOT OPTION_BUILD_DETOURS)
	return()
endif()

# Detour options
option(OPTION_BUILD_DETOURS_FUNCHOOK "FuncHook library detour." ON)

# Detour packages
add_subdirectory(funchook_detour) # FuncHook library
