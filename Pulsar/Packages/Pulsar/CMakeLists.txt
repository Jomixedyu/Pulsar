cmake_minimum_required(VERSION 3.20)
project("PulsarLib")

set(PUBLIC_MODULE
        jxcorlib
        gfx
        gfx-vk
        imgui
        imext
        jaudio
        uinput
        mikktspace
        angelscript
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

add_library(PulsarLib STATIC ${Pulsar_SRC})

target_compile_definitions(PulsarLib PUBLIC PULSAR_GFX_BUILD_VULKAN)
target_link_options(PulsarLib PUBLIC /NATVIS:${CMAKE_CURRENT_LIST_DIR}/Source/src/Pulsar/pulsar.natvis)

if(MSVC)
    target_link_options(PulsarLib INTERFACE "/WHOLEARCHIVE:$<TARGET_FILE:PulsarLib>")
endif()

# mimalloc: disable shared lib and tests for faster build
set(MI_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(MI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory("Source/third/mimalloc")

set(AllModule ${PUBLIC_MODULE})
list(APPEND AllModule ${PRIVATE_MODULE})

foreach (module ${AllModule})
    add_subdirectory("Source/third/${module}")
endforeach ()


target_include_directories(PulsarLib PUBLIC "Source/src")
target_include_directories(PulsarLib PUBLIC "Source/src/Pulsar")

foreach (module ${PUBLIC_MODULE})
    target_link_libraries(PulsarLib PUBLIC ${module})
endforeach ()

foreach (module ${PRIVATE_MODULE})
    target_link_libraries(PulsarLib PRIVATE ${module})
endforeach ()

target_link_libraries(PulsarLib PUBLIC mimalloc-static)
