cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(OpenTimelineIOJS LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -fexperimental-library")

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    add_compile_options(-O0 -g -fprofile-instr-generate -fcoverage-mapping -fwasm-exceptions)
endif()

add_subdirectory(deps)
add_subdirectory(src)
