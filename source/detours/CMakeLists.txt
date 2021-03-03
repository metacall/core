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
	threading
	log
	memory
	portability
	adt
	dynlink
	detour
)

set(DETOUR_MODULE_INCLUDES)

foreach(DETOUR_MODULE ${DETOUR_MODULE_NAMES})
	get_target_property(INCLUDES ${DETOUR_MODULE} INCLUDE_DIRECTORIES)
	list(APPEND DETOUR_MODULE_INCLUDES ${INCLUDES})
endforeach()

# Detour packages
add_subdirectory(funchook_detour) # FuncHook library
