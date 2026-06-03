cmake_minimum_required(VERSION 3.20)
project("PulsarEd")

set(CMAKE_CXX_STANDARD 26)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options("/utf-8")
endif ()

if (WIN32)
    add_compile_definitions(WIN32)
endif ()

file(GLOB_RECURSE Pulsared_SRC "Source/src/*.h" "Source/src/*.hpp" "Source/src/*.cpp")

add_executable(${PROJECT_NAME} ${Pulsared_SRC})

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /bigobj)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "Source/src")
target_include_directories(${PROJECT_NAME} PUBLIC "Source/src/PulsarEd")

target_link_libraries(${PROJECT_NAME} PUBLIC Pulsar)
target_compile_definitions(${PROJECT_NAME} PRIVATE API_EXPORTS)

set(PUBLIC_MODULE
        fbxsdk
        psc
        assimp
)
set(PRIVATE_MODULE
)

# assimp options (must be set before add_subdirectory)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(AllModule ${PUBLIC_MODULE})
list(APPEND AllModule ${PRIVATE_MODULE})

foreach (module ${AllModule})
    add_subdirectory("Source/third/${module}")
endforeach ()

foreach (module ${PUBLIC_MODULE})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${module})
endforeach ()

foreach (module ${PRIVATE_MODULE})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${module})
endforeach ()
