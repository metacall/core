# Check if detours are enabled
if(NOT OPTION_BUILD_DETOURS)
	return()
endif()

# TODO: Disable by default detours funchook plugin until win32 works correctly
if(WIN32)
	set(OPTION_BUILD_DETOURS_FUNCHOOK_DEFAULT OFF)
else()
	set(OPTION_BUILD_DETOURS_FUNCHOOK_DEFAULT ON)
endif()

# Detour options
option(OPTION_BUILD_DETOURS_FUNCHOOK "FuncHook library detour." ${OPTION_BUILD_DETOURS_FUNCHOOK_DEFAULT})

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
