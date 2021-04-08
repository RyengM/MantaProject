cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME Renderer)
project(${PROJECT_NAME} LANGUAGES CXX)

OPTION(TBB "Use multi-thread kernels using Intels TBB" OFF)

include_directories("../ThirdParty/OpenGL/include")
include_directories("../Mantaflow/source")
include_directories("../Mantaflow/source/pwrapper")
include_directories("../Mantaflow/source/util")
include_directories("../Mantaflow/source/fileio")
include_directories("../Mantaflow/source/scenes")

# the path is based on module vcxproj
link_libraries("../../ThirdParty/OpenGL/lib/glfw3")
link_libraries("../../Bin/lib/smoke")
link_libraries("../../Bin/lib/manta_static")
link_libraries("../../Bin/lib/zlib")
if (TBB)
	link_libraries("C:/Program Files/Intel/TBB/lib/intel64/vc14/tbb12.lib")
endif()

file(GLOB_RECURSE HEADER_FILES "Source/*.h")
file(GLOB_RECURSE SOURCE_FILES "Source/*.cpp" "Source/*.c")

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/Bin)

set(CMAKE_COMMAND xcopy /I /E /Y /D "\"${CMAKE_SOURCE_DIR}/Renderer/Shader\""  "\"${CMAKE_SOURCE_DIR}/Bin/$(Configuration)/Shader\"")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND})
set(CMAKE_COMMAND xcopy /I /E /Y /D "\"${CMAKE_SOURCE_DIR}/Renderer/Texture\""  "\"${CMAKE_SOURCE_DIR}/Bin/$(Configuration)/Texture\"")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND})

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin/$(Configuration)")
