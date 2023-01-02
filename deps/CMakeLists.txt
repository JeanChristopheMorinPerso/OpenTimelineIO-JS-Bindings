set(OTIO_SHARED_LIBS OFF)
set(OTIO_CXX_INSTALL OFF)
set(OTIO_DEPENDENCIES_INSTALL OFF)
set(BUILD_TESTING OFF)

add_subdirectory("OpenTimelineIO")
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/OpenTimelineIO/src
  ${CMAKE_CURRENT_SOURCE_DIR}/OpenTimelineIO/src/deps
  ${CMAKE_CURRENT_SOURCE_DIR}/OpenTimelineIO/src/deps/optional-lite/include
  ${CMAKE_CURRENT_SOURCE_DIR}/OpenTimelineIO/src/deps/rapidjson/include
  ${CMAKE_CURRENT_SOURCE_DIR}/OpenTimelineIO/src/deps/Imath/src
)
