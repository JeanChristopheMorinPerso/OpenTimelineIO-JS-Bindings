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
    -sFORCE_FILESYSTEM=1 \
    --pre-js=${CMAKE_CURRENT_SOURCE_DIR}/pre.js "
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
    string(APPEND JS_LINK_FLAGS "--bind -O0 -g3 -gsource-map -fsanitize=address --source-map-base http://localhost:8000/install/ --profile ")
else()
    string(APPEND JS_LINK_FLAGS "-flto -Os --bind ")
    string(APPEND JS_COMPILE_FLAGS "-flto -Os ")
endif()

message(STATUS "JS_LINK_FLAGS: ${JS_LINK_FLAGS}")
message(STATUS "JS_COMPILE_FLAGS: ${JS_COMPILE_FLAGS}")

# Opentime
set(OPENTIME_SRC ${CMAKE_CURRENT_SOURCE_DIR}/opentime)
set(OPENTIME_DEPS
    ${OPENTIME_SRC}/bindings.cpp
)

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

target_include_directories(opentime-js
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src"
)

add_custom_target(opentime-ts
    ALL
    DEPENDS opentime-js
    COMMAND ${CMAKE_COMMAND} -E echo "Creating TypeScript declarations $<TARGET_FILE_DIR:opentime-js>/$<TARGET_FILE_BASE_NAME:opentime-js>.d.ts"
    COMMAND npx tsembind $<TARGET_FILE:opentime-js> > $<TARGET_FILE_DIR:opentime-js>/$<TARGET_FILE_BASE_NAME:opentime-js>.d.ts
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

# Opentimelineio
set(OPENTIMELINEIO_SRC ${CMAKE_CURRENT_SOURCE_DIR}/opentimelineio)
set(OPENTIMEINEIO_DEPS
    ${OPENTIMELINEIO_SRC}/anyVector.cpp
    ${OPENTIMELINEIO_SRC}/bindings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/errorStatusHandler.cpp
    ${OPENTIMELINEIO_SRC}/utils.cpp
    ${OPENTIMELINEIO_SRC}/imath.cpp
)

add_executable(opentimelineio-js ${OPENTIMELINEIO_SRC}/lib.cpp
    ${OPENTIME_DEPS}
    ${OPENTIMEINEIO_DEPS}
)

set_target_properties(opentimelineio-js
    PROPERTIES
    OUTPUT_NAME opentimelineio
    COMPILE_FLAGS "${JS_COMPILE_FLAGS}"
    LINK_FLAGS "${JS_LINK_FLAGS}"
    SOVERSION "1.0")

target_link_libraries(opentimelineio-js
    PUBLIC OTIO::opentime OTIO::opentimelineio)

target_include_directories(opentimelineio-js
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src"
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src/deps"
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src/deps/optional-lite/include"
)

add_custom_target(opentimelineio-ts
    ALL
    DEPENDS opentimelineio-js
    COMMAND ${CMAKE_COMMAND} -E echo "Creating TypeScript declarations $<TARGET_FILE_DIR:opentimelineio-js>/$<TARGET_FILE_BASE_NAME:opentimelineio-js>.d.ts"
    COMMAND npx tsembind $<TARGET_FILE:opentimelineio-js> > $<TARGET_FILE_DIR:opentimelineio-js>/$<TARGET_FILE_BASE_NAME:opentimelineio-js>.d.ts
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

# Install
install(TARGETS opentime-js DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES $<TARGET_FILE_DIR:opentime-js>/$<TARGET_FILE_BASE_NAME:opentime-js>.d.ts DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS opentimelineio-js DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES $<TARGET_FILE_DIR:opentimelineio-js>/$<TARGET_FILE_BASE_NAME:opentimelineio-js>.d.ts DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_BINARY_DIR}/src/opentime.wasm DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_BINARY_DIR}/src/opentimelineio.wasm DESTINATION ${CMAKE_INSTALL_PREFIX})
if (CMAKE_BUILD_TYPE MATCHES Debug)
    install(FILES ${CMAKE_BINARY_DIR}/src/opentime.wasm.map DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_BINARY_DIR}/src/opentimelineio.wasm.map DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
