
cmake_minimum_required(VERSION 3.8)
project("PulsarEditor")
set(CMAKE_CXX_STANDARD 23)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options("/utf-8")
endif ()

add_subdirectory("../Pulsar" Pulsar.out)
add_subdirectory("./third/psc")

# set assimp
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_INSTALL_PDB OFF)
set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
add_subdirectory("./third/assimp")

add_subdirectory("./third/fbxsdk")

#add_library(${PROJECT_NAME} STATIC ${Pulsared_SRC})
file(GLOB_RECURSE Pulsared_SRC "src/*.h" "src/*.hpp" "src/*.cpp" "include/*.h" "include/*.hpp")
add_executable(${PROJECT_NAME} ${Pulsared_SRC})

target_link_libraries(${PROJECT_NAME} PUBLIC fbxsdk)

target_include_directories(${PROJECT_NAME} PUBLIC "./include")
target_include_directories(${PROJECT_NAME} PRIVATE "./include/PulsarEd")

message(${CMAKE_DEBUG_POSTFIX})

#pulsar
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/Pulsar.out")
message("${CMAKE_CURRENT_BINARY_DIR}/Pulsar.out")
target_link_libraries(${PROJECT_NAME} PUBLIC Pulsar)
target_link_options(${PROJECT_NAME} PUBLIC "/WHOLEARCHIVE:Pulsar${CMAKE_DEBUG_POSTFIX}")

target_link_libraries(${PROJECT_NAME} PUBLIC psc)


target_link_libraries(${PROJECT_NAME} PUBLIC assimp)
target_include_directories(${PROJECT_NAME} PRIVATE "./third/assimp/include")

