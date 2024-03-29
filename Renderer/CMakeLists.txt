cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME Renderer)
project(${PROJECT_NAME} LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

OPTION(TBB "Use multi-thread kernels using Intels TBB" OFF)

include_directories("../ThirdParty/OpenGL/include")
include_directories("../ThirdParty/Assimp")
include_directories("../Mantaflow/source")
include_directories("../Mantaflow/source/pwrapper")
include_directories("../Mantaflow/source/util")
include_directories("../Mantaflow/source/fileio")
include_directories("../Mantaflow/source/scenes")

# the path is based on module vcxproj
link_libraries("../../ThirdParty/OpenGL/lib/glfw3")
link_libraries("../../ThirdParty/Assimp/lib/assimp")
link_libraries("../../Bin/lib/smoke")
link_libraries("../../Bin/lib/manta_static")
link_libraries("../../Bin/lib/zlib")
if (TBB)
	link_libraries("C:/Program Files/Intel/TBB/lib/intel64/vc14/tbb12.lib")
endif()

file(GLOB HEADER_FILES "Source/*.h")
file(GLOB SOURCE_FILES "Source/*.cpp" "Source/*.c")
file(GLOB IMGUI_FILES "Source/Imgui/*.cpp", "Source/Imgui/*.h")
source_group("Imgui" FILES ${IMGUI_FILES})

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${IMGUI_FILES})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/Bin)

set(CMAKE_COMMAND xcopy /I /E /Y /D "\"${CMAKE_SOURCE_DIR}/Renderer/Shader\""  "\"${CMAKE_SOURCE_DIR}/Bin/$(Configuration)/Shader\"")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND})
set(CMAKE_COMMAND xcopy /I /E /Y /D "\"${CMAKE_SOURCE_DIR}/Renderer/Texture\""  "\"${CMAKE_SOURCE_DIR}/Bin/$(Configuration)/Texture\"")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND})
set(CMAKE_COMMAND xcopy /I /E /Y /D "\"${CMAKE_SOURCE_DIR}/Renderer/Model\""  "\"${CMAKE_SOURCE_DIR}/Bin/$(Configuration)/Model\"")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND})

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin/$(Configuration)")