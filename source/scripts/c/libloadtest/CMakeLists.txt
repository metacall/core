#
# Configure C project
#

c_project(libloadtest 0.1.0)

# Build the library
add_library(c-libloadtest-shared SHARED
	source/libloadtest.cpp
	source/libloadtest.h
)
