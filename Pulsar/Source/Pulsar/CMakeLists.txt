
cmake_minimum_required (VERSION 3.8)
project("Pulsar")
set(CMAKE_CXX_STANDARD 23)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
endif()

option(${PROJECT_NAME}_BUILD_EXECUTABLE "enable executable mode")

file(GLOB_RECURSE Pulsar_SRC "src/*.h" "src/*.hpp" "src/*.cpp" "include/*.h" "include/*.hpp")

include_directories("../ThirdParty/jxcorlib")

add_subdirectory("../ThirdParty/jxcorlib/CoreLib" jxcorlib.out)
add_subdirectory("../ThirdParty/jxcorlib/CoreLib.Serialization" jxcorlib.ser.out)
add_subdirectory("../ThirdParty/jxcorlib/CoreLib.Math" jxcorlib.math)
add_subdirectory("../ThirdParty/jxcorlib/CoreLib.Platform" jxcorlib.platform.out)

add_library(${PROJECT_NAME} STATIC ${Pulsar_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC "include")
target_include_directories(${PROJECT_NAME} PRIVATE "include/Pulsar")
target_include_directories(${PROJECT_NAME} PUBLIC "..")

target_link_libraries(${PROJECT_NAME} PUBLIC
	jxcorlib
	jxcorlib.ser
	jxcorlib.math
	jxcorlib.platform
	glfw3_mt
	imgui
	PRIVATE
	glad
)
