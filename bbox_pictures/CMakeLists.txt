cmake_minimum_required(VERSION 3.14)
project(bbox_photos)

set(CMAKE_CXX_STANDARD 14)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS})

add_executable(bbox_photos main.cpp)

target_link_libraries( bbox_photos ${OpenCV_LIBS} )