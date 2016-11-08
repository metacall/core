# dotnet.exe publish {source_path}/  -o {output_path}
if(NOT OPTION_BUILD_PLUGINS OR NOT OPTION_BUILD_PLUGINS_CS)
    return()
endif()

execute_process(COMMAND dotnet -v
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  RESULT_VARIABLE dotnet_result
  OUTPUT_VARIABLE dotnet_output)
  


if(dotnet_result EQUAL 0)
	set(netcore_build_script "dotnet restore ${CMAKE_CURRENT_SOURCE_DIR}/source/\ndotnet publish ${CMAKE_CURRENT_SOURCE_DIR}/source/ -o  ${PROJECT_BINARY_DIR}/")
	file(WRITE "${PROJECT_BINARY_DIR}/build_dotnet.bat" "${netcore_build_script}")
	file(WRITE "${PROJECT_BINARY_DIR}/build_dotnet.sh" "#!/bin/bash\n${netcore_build_script}")

endif()