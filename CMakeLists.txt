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

find_package(DanLib REQUIRED)

include_directories(${DanLib_INCLUDE_DIR})

find_package(SFML 2 COMPONENTS system window graphics REQUIRED)

include_directories(${SFML_INCLUDE_DIR})

find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

message(" !!!!!!   ${Boost_INCLUDE_DIRS} ")

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

find_package(Threads)

include_directories(Modules/imgui)
include_directories(Modules/imgui-sfml)
add_subdirectory(Modules/fmt)
include_directories(Modules/fmt/include)

include_directories(.)

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

add_library(
  SpaceTeam
  SpaceTeam/Analog.cpp
  SpaceTeam/Digital.cpp
  SpaceTeam/Game.cpp
  SpaceTeam/Input.cpp
  SpaceTeam/Momentary.cpp
  )

target_link_libraries(
  SpaceTeam
  fmt::fmt
  )

add_executable(
  SpaceTeamServer
  Server/Server.cpp
  )

target_link_libraries(
  SpaceTeamServer
  SpaceTeam
  DanLibTcp
  ${CMAKE_THREAD_LIBS_INIT}
  )

add_executable(
  SpaceTeamGui
  Gui/Main.cpp
  )

target_link_libraries(
  SpaceTeamGui
  SpaceTeam
  imgui-sfml
  DanLibTcp
  ${SFML_LIBRARIES}
  ${OpenGL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  )
