
include(FetchContent)

FetchContent_Declare(backward-cpp
  GIT_REPOSITORY https://github.com/bombela/backward-cpp
  GIT_TAG v1.6
)

FetchContent_MakeAvailable(backward-cpp)

find_package(Backward)
