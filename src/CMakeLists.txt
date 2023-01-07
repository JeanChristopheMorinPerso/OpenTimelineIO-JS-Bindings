set(OTIO_EMSDK_PATH "" CACHE PATH "Path to EMSDK (e.g. 'D:/Projects/emsdk').")
# if (OTIO_EMSCRIPTEN_INSTALL)
#     if (EXISTS "${OTIO_EMSDK_PATH}")
#         set(EMSDK_PATH ${OTIO_EMSDK_PATH})
#     elseif (EXISTS $ENV{EMSDK})
#         set(EMSDK_PATH $ENV{EMSDK})
#     endif()
#     if (EMSDK_PATH)
#         set(CMAKE_TOOLCHAIN_FILE "${EMSDK_PATH}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
#     else()
#         message("The EMSDK path was not supplied, disabling OTIO_EMSCRIPTEN_INSTALL")
#         set(OTIO_EMSCRIPTEN_INSTALL OFF)
#     endif()
# endif()

message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "EMSDK_PATH: ${EMSDK_PATH}")
message(STATUS "EMSDK: $ENV{EMSDK}")

# Linker flags
set(JS_LINK_FLAGS "")
if (EXTERNAL_LINK_FLAGS)
    string(APPEND JS_LINK_FLAGS "${EXTERNAL_LINK_FLAGS} ")
endif()

string(APPEND JS_LINK_FLAGS " -I${CMAKE_SOURCE_DIR}/deps/OpenTimelineIO/src \
    -sWASM=1 \
    -sMODULARIZE=1 \
    -sUSE_ES6_IMPORT_META=0 \
    -sEXPORT_NAME='OpenTimelineIO' \
    -sMEMORY_GROWTH_LINEAR_STEP=32MB \
    -sALLOW_MEMORY_GROWTH=1 \
    -sFORCE_FILESYSTEM=1 "
)

# Support exceptions
# string(APPEND JS_LINK_FLAGS "-sNO_DISABLE_EXCEPTION_CATCHING=0")
# TODO: This is not yet supported in all environments. See https://webassembly.org/roadmap/.
# We might need to use sNO_DISABLE_EXCEPTION_CATCHING instead... Which would be sad.
string(APPEND JS_LINK_FLAGS "-fwasm-exceptions ")

# ASSERTIONS will make sure we have information attached to the WebAssembly.Exception objects.
# For example tags (Exception types), exception message and stack trace.
string(APPEND JS_LINK_FLAGS "-sASSERTIONS ")

# Compiler flags
set(JS_COMPILE_FLAGS "")
if (EXTERNAL_COMPILE_FLAGS)
    string(APPEND JS_COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS} ")
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE MATCHES Debug)
    string(APPEND JS_LINK_FLAGS "--bind -g")
else()
    string(APPEND JS_LINK_FLAGS "-flto -Os --bind ")
    string(APPEND JS_COMPILE_FLAGS "-flto -Os ")
endif()

message(STATUS "JS_LINK_FLAGS: ${JS_LINK_FLAGS}")
message(STATUS "JS_COMPILE_FLAGS: ${JS_COMPILE_FLAGS}")

# Opentime
set(OPENTIME_SRC ${CMAKE_CURRENT_SOURCE_DIR}/opentime)
set(OPENTIME_DEPS ${OPENTIME_SRC}/bindings.cpp)

add_executable(opentime-js ${OPENTIME_SRC}/lib.cpp
    ${OPENTIME_DEPS})

set_target_properties(opentime-js
    PROPERTIES
    OUTPUT_NAME opentime
    COMPILE_FLAGS "${JS_COMPILE_FLAGS}"
    LINK_FLAGS "${JS_LINK_FLAGS}"
    SOVERSION "1.0")

target_link_libraries(opentime-js
    PUBLIC OTIO::opentime)

target_include_directories(opentime-js PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src")

# Opentimelineio
set(OPENTIMELINEIO_SRC ${CMAKE_CURRENT_SOURCE_DIR}/opentimelineio)
set(OPENTIMEINEIO_DEPS
    ${OPENTIMELINEIO_SRC}/bindings.cpp
    ${OPENTIMELINEIO_SRC}/errorStatusHandler.cpp
    ${OPENTIMELINEIO_SRC}/anyVector.cpp
)

add_executable(opentimelineio-js ${OPENTIMELINEIO_SRC}/lib.cpp
    ${OPENTIMEINEIO_DEPS})

set_target_properties(opentimelineio-js
    PROPERTIES
    OUTPUT_NAME opentimelineio
    COMPILE_FLAGS "${JS_COMPILE_FLAGS}"
    LINK_FLAGS "${JS_LINK_FLAGS}"
    SOVERSION "1.0")

target_link_libraries(opentimelineio-js
    PUBLIC OTIO::opentimelineio)

target_include_directories(opentimelineio-js
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src"
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src/deps"
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src/deps/optional-lite/include"
)

# Install
install(TARGETS opentime-js DESTINATION .)
install(TARGETS opentimelineio-js DESTINATION .)
install(FILES ${CMAKE_BINARY_DIR}/src/opentime.wasm DESTINATION .)
install(FILES ${CMAKE_BINARY_DIR}/src/opentimelineio.wasm DESTINATION .)
