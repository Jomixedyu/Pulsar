
cmake_minimum_required(VERSION 3.8)
project("Pulsar")

set(PUBLIC_MODULE
        jxcorlib
        glfw
        gfx
        gfx-vk
        imgui
        imext
        jaudio
)
set(PRIVATE_MODULE
        DirectXTex
        astc
)

set(CMAKE_CXX_STANDARD 23)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options("/utf-8")
endif ()

option(${PROJECT_NAME}_BUILD_EXECUTABLE "enable executable mode" OFF)

file(GLOB_RECURSE Pulsar_SRC "src/*.h" "src/*.hpp" "src/*.cpp" "include/*.h" "include/*.hpp" "include/*.natvis")

if (${PROJECT_NAME}_BUILD_EXECUTABLE)
    add_executable(${PROJECT_NAME} ${Pulsar_SRC} "main.cpp")
else ()
    add_library(${PROJECT_NAME} ${Pulsar_SRC})
endif ()
target_compile_definitions(${PROJECT_NAME} PUBLIC PULSAR_GFX_BUILD_VULKAN)
target_link_options(${PROJECT_NAME} PUBLIC /NATVIS:${CMAKE_CURRENT_LIST_DIR}/include/Pulsar/pulsar.natvis)

set(AllModule ${PUBLIC_MODULE})
list(APPEND AllModule ${PRIVATE_MODULE})

foreach (module ${AllModule})
    add_subdirectory("./third/${module}")
endforeach ()


target_include_directories(${PROJECT_NAME} PUBLIC "./include")
target_include_directories(${PROJECT_NAME} PRIVATE "./include/${PROJECT_NAME}")

foreach (module ${PUBLIC_MODULE})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${module})
endforeach ()

foreach (module ${PRIVATE_MODULE})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${module})
endforeach ()

