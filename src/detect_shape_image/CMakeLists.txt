cmake_minimum_required(VERSION 2.8)
project(detect_shape_image)
find_package(OpenCV REQUIRED)
add_executable(detect_shape_image detect_shape_image.cpp)
target_link_libraries(detect_shape_image ${OpenCV_LIBS})

