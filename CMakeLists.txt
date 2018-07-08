cmake_minimum_required(VERSION 3.10)
project(opencv_video_server)

set(CMAKE_CXX_STANDARD 11)

find_package( OpenCV REQUIRED )
find_library( OpenCV REQUIRED )

add_executable(opencv_video_server main.cpp)
target_link_libraries(opencv_video_server opencv_core opencv_highgui opencv_videoio)