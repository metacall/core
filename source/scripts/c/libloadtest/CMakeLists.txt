#
# Configure C project
#

c_project(libloadtest 0.1.0)

# Build the library
set(target c-libloadtest-shared)

add_library(${target} SHARED
	source/libloadtest.cpp
	source/libloadtest.h
)

set_property(TARGET ${target} PROPERTY CXX_STANDARD 11)
