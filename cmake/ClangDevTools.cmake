# See FindClangFormat.cmake
# Variables of interest on this file: ${CLANG_FORMAT_VERSION} and ${CLANG_FORMAT_EXECUTABLE}

# Get only C/C++ files for now
file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.hpp *.h *.c *.cc *.hh *.cxx)

# clang-tidy not implemented yet
#add_custom_target(
#        clang-tidy
#        COMMAND /usr/bin/clang-tidy
#        ${ALL_SOURCE_FILES}
#        -config=''
#        --
#        -std=c++11
#        ${INCLUDE_DIRECTORIES}
#)

add_custom_target(
        clang-format
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
        -style=file
        -i
        ${ALL_SOURCE_FILES}
)
