include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GoLang.cmake)

# add_go_library(go_port SHARED
# 	source/go_port.go
# )

add_go_executable(go_port
	source/go_port.go
)
