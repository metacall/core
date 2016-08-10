
# Execute cmake_uninstall.cmake wrapper that allows to pass both DESTDIR and COMPONENT environment variable

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY
)

execute_process(
	COMMAND ${CMAKE_COMMAND} -DCOMPONENT=$ENV{COMPONENT} -P cmake_uninstall.cmake
)
