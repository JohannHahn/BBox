cmake_minimum_required(VERSION 3.14)
project(bbox)

set(CMAKE_CXX_STANDARD 14)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS})

add_executable(bbox main.cpp)

target_link_libraries( bbox ${OpenCV_LIBS} )