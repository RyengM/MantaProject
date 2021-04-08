cmake_minimum_required(VERSION 3.15)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Cmake/")

# unicode
add_definitions(-DUNICODE -D_UNICODE)
remove_definitions (-D_MBCS)

set(PROJECT_NAME MantaflowProject)
project(${PROJECT_NAME})

add_subdirectory(Renderer)
add_subdirectory(Mantaflow)