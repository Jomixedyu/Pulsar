
cmake_minimum_required (VERSION 3.8)

project ("jxcorlibTest")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/utf-8")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_compile_options("-g")
	add_compile_options("-fpermissive")
endif()


set(CMAKE_CXX_STANDARD 23)

include_directories(".")

add_subdirectory("./CoreLib")
add_subdirectory("./CoreLib.Serialization")
add_subdirectory("./CoreLib.Math")
add_subdirectory("./CoreLib.Platform")
add_subdirectory("./Example")

aux_source_directory(./ DIR_SRCS)

add_executable (${PROJECT_NAME} ${DIR_SRCS})

target_link_libraries(${PROJECT_NAME} "jxcorlib")
target_link_libraries(${PROJECT_NAME} "jxcorlib.ser")
target_link_libraries(${PROJECT_NAME} "jxcorlib.math")
target_link_libraries(${PROJECT_NAME} "jxcorlib.platform")
target_link_libraries(${PROJECT_NAME} "Example")
