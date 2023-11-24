
cmake_minimum_required (VERSION 3.13)

project ("psc")
set(CMAKE_CXX_STANDARD 23)


cmake_policy(SET CMP0079 NEW)


set(ENABLE_OPT OFF)
add_subdirectory("src/glslang")
add_subdirectory("src/psc")
add_subdirectory("src/spirv-cross")


target_include_directories(${PROJECT_NAME} PUBLIC "./include")
target_include_directories(${PROJECT_NAME} PRIVATE "./include/psc")

#target_include_directories(${PROJECT_NAME} PUBLIC "./src/glslang")

target_link_libraries(${PROJECT_NAME} PRIVATE "GenericCodeGen" "MachineIndependent" "glslang" "HLSL" "OSDependent" "OGLCompiler" "SPIRV")
target_link_libraries(${PROJECT_NAME} PRIVATE "spirv-cross-core" "spirv-cross-glsl")
