cmake_minimum_required(VERSION 3.0)

#fixes wierd library shortname isssue
cmake_policy( SET CMP0060 NEW )

project(SpaceTeamGui)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_C_COMPILER /usr/local/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/local/bin/g++)

set(MODULE_NAME SpaceTeamGui)
add_definitions(-DMODULE_NAME=\"SpaceTeamGui\")

set(OpenGL_LIBRARIES "GL")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a")
add_definitions(-std=c++2a)
add_definitions(-Wall)
add_definitions(-Wpedantic)
add_definitions(-fPIC)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(SFML 2 COMPONENTS system window graphics REQUIRED)

include_directories(${SFML_INCLUDE_DIR})

include_directories(Modules/imgui)
include_directories(Modules/imgui-sfml)

find_package(DanLib REQUIRED)

include_directories(${DanLib_INCLUDE_DIR})

include_directories(src)

set(CMAKE_DEBUG_POSTFIX d)

message("SFML ==== ${SFML_LIBRARIES}   ${SFML_INCLUDE_DIR}")

add_library(
  imgui-sfml
  Modules/imgui/imgui.cpp
  Modules/imgui/imgui_draw.cpp
  Modules/imgui/imgui_widgets.cpp
  Modules/imgui/imgui_demo.cpp
  Modules/imgui-sfml/imgui-SFML.cpp
  )

target_link_libraries(
  imgui-sfml
  ${SFML_LIBRARIES}
  )

add_executable(
  SpaceTeamGui
  src/Main.cpp
  )

target_link_libraries(
  SpaceTeamGui
  imgui-sfml
  ${SFML_LIBRARIES}
  ${OpenGL_LIBRARIES}
  )
