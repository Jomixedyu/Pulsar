cmake_minimum_required (VERSION 3.23)

project ("jaudio")

set(CMAKE_CXX_STANDARD 23)

set(JAUDIO_SRC_LIST
"./src/jaudio/AudioClip.cpp"
"./src/jaudio/AudioDevice.cpp"
"./src/jaudio/AudioListener3D.cpp"
"./src/jaudio/AudioSource3D.cpp"
)

add_library(jaudio ${JAUDIO_SRC_LIST})


get_target_property(target_type jaudio TYPE)
if(target_type STREQUAL "SHARED_LIBRARY")
	target_compile_definitions(${PROJECT_NAME} PUBLIC JAUDIO_BUILD_SHARED)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "./include")
target_include_directories(${PROJECT_NAME} PRIVATE "./include/jaudio")
target_compile_definitions(${PROJECT_NAME} PRIVATE JAUDIO_EXPORT_API)

target_include_directories(${PROJECT_NAME} PRIVATE "./src/al")
target_link_directories(${PROJECT_NAME} PUBLIC "./src/lib/Win64")
target_link_libraries(${PROJECT_NAME} PUBLIC "OpenAL32")

