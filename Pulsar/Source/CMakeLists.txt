
cmake_minimum_required (VERSION 3.8)

project ("PulsarLauncher")
set(CMAKE_CXX_STANDARD 23)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
endif()


add_subdirectory("./ThirdParty/jxcorlib/CoreLib")
add_subdirectory("./ThirdParty/jxcorlib/CoreLib.Serialization")
add_subdirectory("./ThirdParty/jxcorlib/CoreLib.Math")
add_subdirectory("./ThirdParty/jxcorlib/CoreLib.Platform")
add_subdirectory("./ThirdParty")
add_subdirectory("./Pulsar")
add_subdirectory("./PulsarEd")

aux_source_directory(./ DIR_SRCS)
message(${DIR_SRCS})
add_executable (${PROJECT_NAME} ${DIR_SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC ".")
target_include_directories(${PROJECT_NAME} PUBLIC "./ThirdParty")

target_link_directories(${PROJECT_NAME} PUBLIC "./ThirdParty")
target_link_directories(${PROJECT_NAME} PUBLIC "./ThirdParty/glfw")

target_link_libraries(${PROJECT_NAME} jxcorlib)
target_link_libraries(${PROJECT_NAME} jxcorlib.ser)
target_link_libraries(${PROJECT_NAME} jxcorlib.math)
target_link_libraries(${PROJECT_NAME} jxcorlib.platform)
target_link_libraries(${PROJECT_NAME} Pulsar)
target_link_libraries(${PROJECT_NAME} PulsarEditor)
target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} glfw3_mt)
target_link_libraries(${PROJECT_NAME} imgui)
target_link_libraries(${PROJECT_NAME} assimp-vc143-mt)


#target_link_directories(${PROJECT_NAME} PUBLIC "include")