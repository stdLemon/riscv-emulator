cmake_minimum_required(VERSION 3.0.0)
project(riscv-emulator VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -ggdb)

