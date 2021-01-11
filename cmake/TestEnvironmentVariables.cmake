#
#	Test Environment Variables by Parra Studios
#	Utility for defining cross-platform environment variables in tests.
#
#	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
#

if(TEST_ENVIRONMENT_VARIABLES_FOUND)
	return()
endif()

set(TEST_ENVIRONMENT_VARIABLES_FOUND YES)

get_filename_component(TEST_ENVIRONMENT_VARIABLES_PATH ${CMAKE_CURRENT_LIST_FILE} PATH)

function(test_environment_variables target args)

	set(env_vars ${ARGN})

	if(MSVC)

		set(MSVC_LOCAL_DEBUGGER_ENVIRONMENT ${env_vars})
		set(MSVC_COMMAND_LINE_ARGS ${args})

		set(OUTPUT_DIRECTORY_DIR "$(OutDir)")
		string(CONFIGURE "${MSVC_LOCAL_DEBUGGER_ENVIRONMENT}" MSVC_LOCAL_DEBUGGER_ENVIRONMENT @ONLY)

		string(REPLACE ";" "\n" MSVC_LOCAL_DEBUGGER_ENVIRONMENT "${MSVC_LOCAL_DEBUGGER_ENVIRONMENT}")

		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(MSVC_PLATFORM "x64")
		else()
			set(MSVC_PLATFORM "Win32")
		endif()

		if(MSVC_VERSION GREATER 1500)

			if(MSVC_VERSION GREATER 1800)
				set(MSVC_TOOLS_VERSION "12.0")
			elseif(MSVC_VERSION GREATER 1500)
				set(MSVC_TOOLS_VERSION "4.0")
			elseif(MSVC_VERSION GREATER 1400)
				set(MSVC_TOOLS_VERSION "3.5")
			elseif(MSVC_VERSION GREATER 1301)
				set(MSVC_TOOLS_VERSION "3.0")
			elseif(MSVC_VERSION GREATER 1300)
				set(MSVC_TOOLS_VERSION "1.1")
			else()
				set(MSVC_TOOLS_VERSION "1.0")
			endif()

			set(MSVC_LOCAL_PROPERTY_GROUPS "")

			set(MSVC_PROPERTY_GROUP_TEMPLATE " \
				<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='@MSVC_BUILD_CONFIG@|@MSVC_PLATFORM@'\">\n \
					<LocalDebuggerEnvironment>\n@MSVC_LOCAL_DEBUGGER_ENVIRONMENT@\n$(LocalDebuggerEnvironment)\n \
					</LocalDebuggerEnvironment>\n \
					<LocalDebuggerWorkingDirectory>$(OutDir)</LocalDebuggerWorkingDirectory>\n \
					<LocalDebuggerCommandArguments>@MSVC_COMMAND_LINE_ARGS@</LocalDebuggerCommandArguments>\n \
					<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>\n \
				</PropertyGroup>\n \
			")

			set(MSVC_PROJECT_OUTPUT_EXT "vcxproj")

			foreach(MSVC_BUILD_CONFIG ${CMAKE_CONFIGURATION_TYPES})

				string(CONFIGURE ${MSVC_PROPERTY_GROUP_TEMPLATE} MSVC_PROPERTY_GROUP)

				set(MSVC_LOCAL_PROPERTY_GROUPS "${MSVC_LOCAL_PROPERTY_GROUPS}${MSVC_PROPERTY_GROUP}\n")

			endforeach()

		else()

			if(MSVC_VERSION EQUAL 2000)
				set(MSVC_VERSION_NUMBER "15.00")
			elseif(MSVC_VERSION EQUAL 1900)
				set(MSVC_VERSION_NUMBER "14.00")
			elseif(MSVC_VERSION EQUAL 1800)
				set(MSVC_VERSION_NUMBER "12.00")
			elseif(MSVC_VERSION EQUAL 1700)
				set(MSVC_VERSION_NUMBER "11.00")
			elseif(MSVC_VERSION EQUAL 1600)
				set(MSVC_VERSION_NUMBER "10.00")
			elseif(MSVC_VERSION EQUAL 1500)
				set(MSVC_VERSION_NUMBER "9.00")
			elseif(MSVC_VERSION EQUAL 1400)
				set(MSVC_VERSION_NUMBER "8.00")
			elseif(MSVC_VERSION EQUAL 1310)
				set(MSVC_VERSION_NUMBER "7.01")
			elseif(MSVC_VERSION EQUAL 1300)
				set(MSVC_VERSION_NUMBER "7.00")
			elseif(MSVC_VERSION EQUAL 1200)
				set(MSVC_VERSION_NUMBER "6.00")
			else()
				set(MSVC_VERSION_NUMBER "5.0")
			endif()

			set(MSVC_LOCAL_CONFIGURATIONS "")

			set(MSVC_LOCAL_CONFIGURATION_TEMPLATE " \
				<Configuration Name=\"@MSVC_BUILD_CONFIG@|@MSVC_PLATFORM@\">\n \
					<DebugSettings\n \
						WorkingDirectory=\"$(OutDir)\"\n \
						CommandArguments=\"@MSVC_COMMAND_LINE_ARGS@\"\n \
						Environment=\"@MSVC_LOCAL_DEBUGGER_ENVIRONMENT@\"\n \
						EnvironmentMerge=\"true\"\n \
					 /> \
				</Configuration>\n \
			")

			set(MSVC_PROJECT_OUTPUT_EXT "vcproj")

			foreach(MSVC_BUILD_CONFIG ${CMAKE_CONFIGURATION_TYPES})

				string(CONFIGURE ${MSVC_LOCAL_CONFIGURATION_TEMPLATE} MSVC_LOCAL_CONFIGURATION)

				set(MSVC_LOCAL_CONFIGURATIONS "${MSVC_LOCAL_CONFIGURATIONS}${MSVC_LOCAL_CONFIGURATION}\n")

			endforeach()

		endif()

		set(MSVC_PROJECT_TEMPLATE "${TEST_ENVIRONMENT_VARIABLES_PATH}/ProjectTemplate.${MSVC_PROJECT_OUTPUT_EXT}.user.in")
		configure_file(${MSVC_PROJECT_TEMPLATE} ${CMAKE_CURRENT_BINARY_DIR}/${target}.${MSVC_PROJECT_OUTPUT_EXT}.user @ONLY)

		set(MSVC_LOCAL_CTEST_ENVIRONMENT ${env_vars})

		set(OUTPUT_DIRECTORY_DIR "${PROJECT_BINARY_DIR}/$<CONFIGURATION>/")
		string(CONFIGURE "${MSVC_LOCAL_CTEST_ENVIRONMENT}" MSVC_LOCAL_CTEST_ENVIRONMENT @ONLY)

		set_property(TEST ${target}
			PROPERTY ENVIRONMENT "${MSVC_LOCAL_CTEST_ENVIRONMENT}"
		)

	else()

		set(MAKE_LOCAL_CTEST_ENVIRONMENT ${env_vars})

		set(OUTPUT_DIRECTORY_DIR "${PROJECT_BINARY_DIR}")
		string(CONFIGURE "${MAKE_LOCAL_CTEST_ENVIRONMENT}" MAKE_LOCAL_CTEST_ENVIRONMENT @ONLY)

		set_property(TEST ${target}
			PROPERTY ENVIRONMENT "${MAKE_LOCAL_CTEST_ENVIRONMENT}"
		)

	endif()

endfunction()
