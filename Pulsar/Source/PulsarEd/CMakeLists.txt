
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


#add_library(${PROJECT_NAME} STATIC ${Pulsared_SRC})
file(GLOB_RECURSE Pulsared_SRC "src/*.h" "src/*.hpp" "src/*.cpp" "include/*.h" "include/*.hpp")
add_executable(${PROJECT_NAME} ${Pulsared_SRC})


target_include_directories(${PROJECT_NAME} PUBLIC "./include")
target_include_directories(${PROJECT_NAME} PRIVATE "./include/PulsarEd")


#fbx
set(FBXSDK_DIR "C:/Program Files/Autodesk/FBX/FBX SDK/2020.0.1")
target_include_directories(${PROJECT_NAME} PUBLIC "${FBXSDK_DIR}/include")
target_link_directories(${PROJECT_NAME} PUBLIC "${FBXSDK_DIR}/lib/vs2017/x64/$<IF:$<CONFIG:Debug>,debug,release>")
target_link_libraries(${PROJECT_NAME} PUBLIC "libfbxsdk-md" "wininet" "libxml2-md" "zlib-md")
file(GENERATE OUTPUT "./gentext.txt" CONTENT "${FBXSDK_DIR}/lib/vs2017/x64/$<IF:$<CONFIG:Debug>,debug,release>")

#pulsar
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/Pulsar.out")
target_link_libraries(${PROJECT_NAME} PUBLIC Pulsar)
target_link_options(${PROJECT_NAME} PUBLIC "/WHOLEARCHIVE:Pulsar")

target_link_libraries(${PROJECT_NAME} PUBLIC psc)


target_link_libraries(${PROJECT_NAME} PUBLIC assimp)
target_include_directories(${PROJECT_NAME} PRIVATE "./third/assimp/include")

