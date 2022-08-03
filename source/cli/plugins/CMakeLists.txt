#Check if extension loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_EXTENSIONS)
	return()
endif()

# Extension sub-projects
add_subdirectory(core_plugin)

# Install plugin directory
install(DIRECTORY ${PROJECT_OUTPUT_DIR}/plugins
	DESTINATION ${INSTALL_LIB}/plugins
	PATTERN "test[-_]*" EXCLUDE
)