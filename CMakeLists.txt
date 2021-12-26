cmake_minimum_required(VERSION 3.10)
project(StereoKitImGui VERSION "1.0.0" LANGUAGES CXX C)

option(SKIG_USE_CIMGUI "Link against CImGui instead of ImGui" OFF)

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
  GIT_TAG v0.3.4
  OPTIONS
    "SK_BUILD_TESTS OFF"
    "SK_BUILD_SHARED_LIBS ON"
    "BUILD_WITH_XCB_HEADERS OFF"
)

list(APPEND SKIG_LIBRARIES "StereoKitC")

# ImGui #

if(SKIG_USE_CIMGUI)
  CPMAddPackage(
    NAME cimgui
    GITHUB_REPOSITORY cimgui/cimgui
    GIT_TAG 1.85
  )

  list(APPEND SKIG_LIBRARIES "cimgui")

  add_compile_definitions(SKIG_USE_CIMGUI)
else()
  include("cmake/ImGui.cmake")

  list(APPEND SKIG_LIBRARIES "ImGui")
endif()

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
  ${SKIG_LIBRARIES}
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
  ${SKIG_LIBRARIES}
  StereoKitImGui
)

# TODO: fix RPATH
