# Check if detours are enabled
if(NOT OPTION_FORK_SAFE OR NOT OPTION_BUILD_DETOURS)
	return()
endif()

# Detour options
option(OPTION_BUILD_DETOURS_FUNCHOOK "FuncHook library detour." ON)

# Define detour modules
set(DETOUR_MODULE_NAMES
	version
	preprocessor
	environment
	format
	log
	memory
	adt
	dynlink
	detour
)

# Detour packages
add_subdirectory(funchook_detour) # FuncHook library
