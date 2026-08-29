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
    string(APPEND JS_LINK_FLAGS "-lembind -O0 -g3 -gsource-map -fsanitize=address --source-map-base http://localhost:8000/install/ --profile ")
else()
    string(APPEND JS_LINK_FLAGS "-flto -Os -lembind ")
    string(APPEND JS_COMPILE_FLAGS "-flto -Os ")
endif()

message(STATUS "JS_LINK_FLAGS: ${JS_LINK_FLAGS}")
message(STATUS "JS_COMPILE_FLAGS: ${JS_COMPILE_FLAGS}")

set(ENSURE_TSD_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/ensure_tsd.cmake")
file(WRITE "${ENSURE_TSD_SCRIPT}" [=[
if(NOT EXISTS "${TSD}")
    file(TOUCH "${STAMP}")
endif()
]=])

function(emit_typescript_declarations target output)
    set(stamp "${CMAKE_CURRENT_BINARY_DIR}/${target}-tsd.stamp")
    if(NOT EXISTS "${stamp}")
        file(TOUCH "${stamp}")
    endif()

    target_link_options(${target} PRIVATE "--emit-tsd" "${output}")
    add_custom_target(${target}-check-tsd
        COMMAND ${CMAKE_COMMAND}
            "-DTSD=${output}"
            "-DSTAMP=${stamp}"
            -P "${ENSURE_TSD_SCRIPT}"
        VERBATIM
    )
    add_dependencies(${target} ${target}-check-tsd)
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${stamp}")
    add_custom_command(TARGET ${target} POST_BUILD
        BYPRODUCTS "${output}"
        COMMAND ${CMAKE_COMMAND} -E true
    )
endfunction()

# Opentime
set(OPENTIME_SRC ${CMAKE_CURRENT_SOURCE_DIR}/opentime)
set(OPENTIME_DEPS
    ${OPENTIME_SRC}/bindings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/errorStatusHandler.cpp
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
    OTIO::opentime OTIO::opentimelineio)

em_link_pre_js(opentime-js "${CMAKE_CURRENT_SOURCE_DIR}/pre.js")

target_include_directories(opentime-js
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src"
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src/deps"
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src/deps/optional-lite/include"
)

set(OPENTIME_TSD "${CMAKE_CURRENT_BINARY_DIR}/opentime.d.ts")
emit_typescript_declarations(opentime-js "${OPENTIME_TSD}")

# Opentimelineio
set(OPENTIMELINEIO_SRC ${CMAKE_CURRENT_SOURCE_DIR}/opentimelineio)
set(OPENTIMEINEIO_DEPS
    ${OPENTIMELINEIO_SRC}/anyVector.cpp
    ${OPENTIMELINEIO_SRC}/bindings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/errorStatusHandler.cpp
    ${OPENTIMELINEIO_SRC}/utils.cpp
    ${OPENTIMELINEIO_SRC}/imath.cpp
    ${OPENTIMELINEIO_SRC}/js_any.cpp
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
    OTIO::opentime OTIO::opentimelineio)

em_link_pre_js(opentimelineio-js "${CMAKE_CURRENT_SOURCE_DIR}/pre.js")

target_include_directories(opentimelineio-js
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src"
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src/deps"
    PRIVATE "${PROJECT_SOURCE_DIR}/deps/OpenTimelineIO/src/deps/optional-lite/include"
)

set(OPENTIMELINEIO_TSD "${CMAKE_CURRENT_BINARY_DIR}/opentimelineio.d.ts")
emit_typescript_declarations(opentimelineio-js "${OPENTIMELINEIO_TSD}")

# Install
install(TARGETS opentime-js DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES "${OPENTIME_TSD}" DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS opentimelineio-js DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES "${OPENTIMELINEIO_TSD}" DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_BINARY_DIR}/src/opentime.wasm DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_BINARY_DIR}/src/opentimelineio.wasm DESTINATION ${CMAKE_INSTALL_PREFIX})
if (CMAKE_BUILD_TYPE MATCHES Debug)
    install(FILES ${CMAKE_BINARY_DIR}/src/opentime.wasm.map DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_BINARY_DIR}/src/opentimelineio.wasm.map DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
