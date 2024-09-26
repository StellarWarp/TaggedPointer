cmake_minimum_required(VERSION 3.28)
project(TaggedPointer)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)
include_directories(test)

file(GLOB_RECURSE SRC_FILES "test/*.cpp")


add_executable(TaggedPointer ${SRC_FILES})
