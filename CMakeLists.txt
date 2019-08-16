cmake_minimum_required(VERSION 3.0)

#fixes wierd library shortname isssue
cmake_policy( SET CMP0060 NEW )

add_definitions(-Wno-psabi)

project(SpaceTeamGui)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

OPTION(BUILD_RPI "Build For Rpi" ON)

if(BUILD_RPI)
  set(CMAKE_C_COMPILER /usr/local/gcc-8.1.0/bin/gcc-8.1.0)
  set(CMAKE_CXX_COMPILER /usr/local/gcc-8.1.0/bin/g++-8.1.0)
else()
  set(CMAKE_C_COMPILER /usr/local/bin/gcc)
  set(CMAKE_CXX_COMPILER /usr/local/bin/g++)
endif()

set(MODULE_NAME SpaceTeamGui)

set(OpenGL_LIBRARIES "GL")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a")
add_definitions(-std=c++2a)
add_definitions(-Wall)
add_definitions(-Wpedantic)
add_definitions(-fPIC)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(SFML 2 COMPONENTS system window graphics REQUIRED)

include_directories(${SFML_INCLUDE_DIR})

find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

message(" !!!!!!   ${Boost_INCLUDE_DIRS} ")

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

if(BUILD_RPI)

  include_directories(SYSTEM Modules/asio/asio/include)

  add_subdirectory(Modules/DanLib EXCLUDE_FROM_ALL)

  include_directories(Modules/DanLib/Tcp)
  include_directories(Modules/DanLib/Utility)
  include_directories(Modules/DanLib/Robot)
else()
  include_directories(Modules/asio/asio/include)

  add_subdirectory(Modules/DanLib EXCLUDE_FROM_ALL)

  include_directories(Modules/DanLib/Tcp)
  include_directories(Modules/DanLib/Utility)
  include_directories(Modules/DanLib/Robot)
endif()


find_library(WIRING_PI wiringPi)

if(WIRING_PI)
  list(APPEND WIRING_PI_LIBRARIES ${WIRING_PI})
endif()

find_package(Threads)

include_directories(Modules/imgui)
include_directories(Modules/imgui-sfml)
add_subdirectory(Modules/fmt)
include_directories(Modules/fmt/include)
include_directories(Modules)

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
  SpaceTeam/Update.cpp
  SpaceTeam/Panel.cpp
  )

add_library(
  SpaceTeamUtility
  Utility/JsonAssembler.cpp
  )

if(BUILD_RPI)
  add_definitions(-DENABLE_HARDWARE)
endif()

target_link_libraries(
  SpaceTeam
  fmt::fmt
  )

if(BUILD_RPI)
  list(APPEND HARDWARE_SOURCE HardwareInterface/I2c.cpp)
  list(APPEND HARDWARE_SOURCE HardwareInterface/Spi.cpp)
else()
  list(APPEND HARDWARE_SOURCE HardwareInterface/MockI2c.cpp)
  list(APPEND HARDWARE_SOURCE HardwareInterface/MockSpi.cpp)
endif()

add_library(
  SpaceTeamHardware
  ${HARDWARE_SOURCE}
  )

target_link_libraries(
  SpaceTeamHardware
  ${WIRING_PI_LIBRARIES}
  )


add_executable(
  Server
  Server/Server.cpp
  )

target_link_libraries(
  Server
  SpaceTeam
  DanLibTcp
  ${CMAKE_THREAD_LIBS_INIT}
  )

add_executable(
  Gui
  Gui/Main.cpp
  )

target_link_libraries(
  Gui
  SpaceTeam
  SpaceTeamUtility
  SpaceTeamHardware
  imgui-sfml
  DanLibTcp
  ${SFML_LIBRARIES}
  ${OpenGL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  )

  add_executable(
    Simulator
    Simulator/Simulator.cpp
    )

  target_link_libraries(
    Simulator
    SpaceTeam
    SpaceTeamUtility
    imgui-sfml
    DanLibTcp
    ${SFML_LIBRARIES}
    ${OpenGL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

add_executable(
  ConfigFileGenerator
  Setup/Setup.cpp
  )

target_link_libraries(
  ConfigFileGenerator
  SpaceTeamHardware
  fmt::fmt
  )

add_executable(
  ToggleInputsPrintAnalog
  TestUtilities/ToggleInputsPrintAnalog.cpp
  )

target_link_libraries(
  ToggleInputsPrintAnalog
  SpaceTeamHardware
  fmt::fmt
  )
