cmake_minimum_required(VERSION 3.8)

project(qtpromise VERSION 0.5.0 LANGUAGES CXX)

find_package(Qt5 5.6.0 REQUIRED COMPONENTS Core)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

add_library(qtpromise INTERFACE)
add_library(qtpromise::qtpromise ALIAS qtpromise)

target_link_libraries(qtpromise INTERFACE Qt5::Core)

target_include_directories(qtpromise INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
