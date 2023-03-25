cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# C is only required because of the em_link_pre_js function.
# TODO: Remove C once https://github.com/emscripten-core/emscripten/pull/19071 gets released.
project(OpenTimelineIOJS LANGUAGES C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -fexperimental-library")

add_subdirectory(deps)
add_subdirectory(src)
