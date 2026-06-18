if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_RS) 
    return()    
endif()

set(target metacall-rust-conversion-test)
message(STATUS "Test ${target}")

include(Warnings)
include(SecurityFlags)

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/source/")

set(sources
    ${source_path}/main.cpp 
    ${source_path}/metacall_rust_conversion_test.cpp
)

set(header_group "Header Files (API)")
set(source_group "Souce files")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$"
    ${header_group} ${headers})
source_group_by_path(${source_path} "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$"
    ${source_group} ${sources})

add_executable(${target} ${sources})
add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

target_include_directories(${target}
	PRIVATE
	${DEFAULT_INCLUDE_DIRECTORIES}
	${PROJECT_BINARY_DIR}/source/include
)

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LIBRARIES}

    GTest

    ${META_PROJECT_NAME}::metacall
)

target_compile_definitions(${target}
    PRIVATE
    ${DEFAULT_COMPILE_DEFINITIONS}    
)

target_compile_options(${target}
    PRIVATE
    ${DEFAULT_COMPILE_OPTIONS}
)

target_compile_features(${target}
    PRIVATE
    cxx_std_17
)

target_link_options(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
add_dependencies(${target} rs_loader)

set_property(TEST ${target} PROPERTY LABELS ${target})

include(TestEnvironmentVariables)

test_environment_variables(${target}
   ""
   ${TESTS_ENVIRONMENT_VARIABLES}
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/source/identity.rs
	DESTINATION ${PROJECT_BINARY_DIR}/scripts)