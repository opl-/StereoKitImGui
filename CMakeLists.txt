cmake_minimum_required(VERSION 3.10)
project(StereoKitImGui VERSION "1.0.0" LANGUAGES CXX C)

# Get dependencies using CPM, this is grabbing StereoKit
# from the Github repository. You may want to consider 
# setting the CPM_SOURCE_CACHE environment variable as
# described by the CPM docs.
# See: https://github.com/cpm-cmake/CPM.cmake
include("cmake/CPM.cmake")

# StereoKitC #

CPMAddPackage(
  NAME StereoKitC
  GITHUB_REPOSITORY maluoi/StereoKit
  GIT_TAG dd1f7f65f33df010c9838a25ffe0b5ad57ae3ae0 # /develop branch
  OPTIONS
    "SK_BUILD_TESTS OFF"
    "SK_BUILD_SHARED_LIBS ON"
    "BUILD_WITH_XCB_HEADERS OFF"
)

CPMAddPackage(
  NAME ImGui
  GITHUB_REPOSITORY ocornut/imgui
  GIT_TAG v1.85
)

# ImGui #

set(IMGUI_SOURCES
  ${ImGui_SOURCE_DIR}/imgui.cpp
  ${ImGui_SOURCE_DIR}/imgui_draw.cpp
  ${ImGui_SOURCE_DIR}/imgui_tables.cpp
  ${ImGui_SOURCE_DIR}/imgui_widgets.cpp
  ${ImGui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

set(IMGUI_PUBLIC_HEADERS
  ${ImGui_SOURCE_DIR}/imconfig.h
  ${ImGui_SOURCE_DIR}/imgui.h
  ${ImGui_SOURCE_DIR}/imgui_internal.h # not actually public, but users might need it
  ${ImGui_SOURCE_DIR}/imstb_rectpack.h
  ${ImGui_SOURCE_DIR}/imstb_textedit.h
  ${ImGui_SOURCE_DIR}/imstb_truetype.h
  ${ImGui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h
)

add_library(ImGui SHARED
  ${IMGUI_PUBLIC_HEADERS}
  ${IMGUI_SOURCES}
)

target_include_directories(ImGui
PUBLIC
  ${ImGui_SOURCE_DIR}
)

set_target_properties(ImGui PROPERTIES
  PUBLIC_HEADER "${IMGUI_PUBLIC_HEADERS}"
)

# StereoKitImGui #

add_library(StereoKitImGui SHARED
  ${CMAKE_CURRENT_LIST_DIR}/include/skig.h
  ${CMAKE_CURRENT_LIST_DIR}/src/skig.cpp
)

set_target_properties(StereoKitImGui PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  PUBLIC_HEADER include/skig.h
)

target_include_directories(StereoKitImGui
PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/include/
PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/src/
  # We need access to sk_gpu.h - give ourselves access to the StereoKit library headers
  ${StereoKitC_SOURCE_DIR}/StereoKitC/libraries/
)

target_link_libraries(StereoKitImGui
  ImGui
  StereoKitC
)

# StereoKitImGui example #

add_executable(StereoKitImGuiExample
  ${CMAKE_CURRENT_LIST_DIR}/example/main.cpp
)

set_target_properties(StereoKitImGuiExample PROPERTIES
  VERSION ${PROJECT_VERSION}
)

target_include_directories(StereoKitImGuiExample
PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/example/
  ${CMAKE_CURRENT_LIST_DIR}/include/
)

target_link_libraries(StereoKitImGuiExample
  ImGui
  StereoKitC
  StereoKitImGui
)

# TODO: fix RPATH
