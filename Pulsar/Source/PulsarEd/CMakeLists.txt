
cmake_minimum_required (VERSION 3.8)
project("PulsarEditor")
set(CMAKE_CXX_STANDARD 23)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
endif()


include_directories("../ThirdParty/jxcorlib")
add_subdirectory("../Pulsar" Pulsar)
add_subdirectory("../ThirdParty" ThirdParty)

#add_library(${PROJECT_NAME} STATIC ${Pulsared_SRC})
file(GLOB_RECURSE Pulsared_SRC "src/*.h" "src/*.hpp" "src/*.cpp" "include/*.h" "include/*.hpp")
add_executable(${PROJECT_NAME} ${Pulsared_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC "include")
target_include_directories(${PROJECT_NAME} PUBLIC "../Pulsar/include")
target_include_directories(${PROJECT_NAME} PRIVATE "include/PulsarEd")
target_include_directories(${PROJECT_NAME} PUBLIC "..")
target_include_directories(${PROJECT_NAME} PUBLIC "../ThirdParty")

target_link_directories(${PROJECT_NAME} PUBLIC "D:/Codes/Pulsar/Pulsar/Source/PulsarEd/build/Pulsar/Debug")
target_link_directories(${PROJECT_NAME} PUBLIC "D:/Codes/Pulsar/Pulsar/Source/ThirdParty")
target_link_directories(${PROJECT_NAME} PUBLIC "D:/Codes/Pulsar/Pulsar/Source/ThirdParty/glfw")
target_link_libraries(${PROJECT_NAME} PUBLIC
	pulsar
	assimp-vc143-mt
	glfw3_mt
	imgui
	PRIVATE
	glad
)