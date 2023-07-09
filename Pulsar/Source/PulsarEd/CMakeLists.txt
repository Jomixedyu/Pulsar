
cmake_minimum_required (VERSION 3.8)
project("PulsarEditor")
set(CMAKE_CXX_STANDARD 23)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
endif()

file(GLOB_RECURSE Pulsared_SRC "src/*.h" "src/*.hpp" "src/*.cpp" "include/*.h" "include/*.hpp")


add_library(${PROJECT_NAME} OBJECT ${Pulsared_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC "include")
target_include_directories(${PROJECT_NAME} PRIVATE "include/Pulsar")

target_link_libraries(${PROJECT_NAME} PUBLIC
	pulsar
)