
cmake_minimum_required (VERSION 3.8)

project ("PulsarEditor")
set(CMAKE_CXX_STANDARD 23)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
endif()

include_directories(".")
include_directories("./ThirdParty")

link_directories("ThirdParty")
link_directories("ThirdParty/glfw")

add_subdirectory("./CoreLib")
add_subdirectory("./CoreLib.Serialization")
add_subdirectory("./CoreLib.Math")
add_subdirectory("./CoreLib.Platform")
add_subdirectory("./ThirdParty")
add_subdirectory("./Pulsar")
add_subdirectory("./PulsarEd")

aux_source_directory(./ DIR_SRCS)

add_executable (${PROJECT_NAME} ${DIR_SRCS})

target_link_libraries(${PROJECT_NAME} jxcorlib)
target_link_libraries(${PROJECT_NAME} jxcorlib.ser)
target_link_libraries(${PROJECT_NAME} jxcorlib.math)
target_link_libraries(${PROJECT_NAME} jxcorlib.platform)
target_link_libraries(${PROJECT_NAME} pulsar)
target_link_libraries(${PROJECT_NAME} pulsared)
target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} glfw3_mt)
target_link_libraries(${PROJECT_NAME} imgui)
target_link_libraries(${PROJECT_NAME} assimp-vc143-mt)


#target_link_directories(${PROJECT_NAME} PUBLIC "include")