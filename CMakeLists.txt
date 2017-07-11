cmake_minimum_required(VERSION 3.7)
project(Mandelbrot)


SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g0")

set(SOURCE_FILES mandelbrot.c)
add_executable(mandelbrot ${SOURCE_FILES})
target_link_libraries(mandelbrot GLEW GLU GL glut)