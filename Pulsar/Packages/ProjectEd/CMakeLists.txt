cmake_minimum_required(VERSION 3.20)
project("ProjectEd")

set(CMAKE_CXX_STANDARD 26)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options("/utf-8")
endif ()

if (WIN32)
    add_compile_definitions(WIN32)
endif ()

add_executable(${PROJECT_NAME} Source/main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC Project PulsarEdLib)
