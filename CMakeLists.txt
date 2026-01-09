cmake_minimum_required(VERSION 3.23)

project(asobi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(thirdParty/mai_vk)
add_subdirectory(thirdParty/assimp)


file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(asobi STATIC ${MY_SOURCES})
add_library(asobi::asobi ALIAS asobi)


target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC EXAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/examples/")
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/")

target_include_directories(${PROJECT_NAME} PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)


target_link_libraries(${PROJECT_NAME} PUBLIC
		mai::mai
		assimp
)

add_subdirectory(examples/helloTriangle)
add_subdirectory(examples/textures)
add_subdirectory(examples/cubemap)
add_subdirectory(examples/tessellation)
add_subdirectory(examples/flipbook_animaition)
