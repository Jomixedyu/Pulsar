cmake_minimum_required(VERSION 3.20)
project("Pulsar")

set(PUBLIC_MODULE
        jxcorlib
        gfx
        gfx-vk
        imgui
        imext
        jaudio
        uinput
        mikktspace
)
set(PRIVATE_MODULE
        DirectXTex
        astc
        box2d
        SDL2lib
        Jolt
)

set(CMAKE_CXX_STANDARD 26)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options("/utf-8")
endif ()

if (WIN32)
    add_compile_definitions(WIN32)
endif ()

file(GLOB_RECURSE Pulsar_SRC "Source/src/*.h" "Source/src/*.hpp" "Source/src/*.cpp" "Source/src/*.natvis")

add_library(${PROJECT_NAME} OBJECT ${Pulsar_SRC})

target_compile_definitions(${PROJECT_NAME} PUBLIC PULSAR_GFX_BUILD_VULKAN)
target_link_options(${PROJECT_NAME} PUBLIC /NATVIS:${CMAKE_CURRENT_LIST_DIR}/Source/src/Pulsar/pulsar.natvis)

# mimalloc: disable shared lib and tests for faster build
set(MI_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(MI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory("Source/third/mimalloc")

set(AllModule ${PUBLIC_MODULE})
list(APPEND AllModule ${PRIVATE_MODULE})

foreach (module ${AllModule})
    add_subdirectory("Source/third/${module}")
endforeach ()


target_include_directories(${PROJECT_NAME} PUBLIC "Source/src")
target_include_directories(${PROJECT_NAME} PUBLIC "Source/src/${PROJECT_NAME}")

foreach (module ${PUBLIC_MODULE})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${module})
endforeach ()

foreach (module ${PRIVATE_MODULE})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${module})
endforeach ()

target_link_libraries(${PROJECT_NAME} PUBLIC mimalloc-static)
