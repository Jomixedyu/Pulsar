
cmake_minimum_required(VERSION 3.20)
project("PulsarEd")

set(PUBLIC_MODULE
        fbxsdk
        psc
)
set(PRIVATE_MODULE
        assimp
)


set(CMAKE_CXX_STANDARD 23)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options("/utf-8")
endif ()


#add_library(${PROJECT_NAME} STATIC ${Pulsared_SRC})
file(GLOB_RECURSE Pulsared_SRC "src/*.h" "src/*.hpp" "src/*.cpp" "include/*.h" "include/*.hpp" "include/*.cpp")
add_executable(${PROJECT_NAME} ${Pulsared_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC "./include")
target_include_directories(${PROJECT_NAME} PRIVATE "./include/PulsarEd")


#link pulsar
set(Pulsar_BUILD_EXECUTABLE OFF)
add_subdirectory("../Pulsar" Pulsar.out)
target_link_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/Pulsar.out")
target_link_libraries(${PROJECT_NAME} PUBLIC Pulsar)
target_link_options(${PROJECT_NAME} PUBLIC "/WHOLEARCHIVE:Pulsar$<$<CONFIG:Debug>:${CMAKE_DEBUG_POSTFIX}>")


set(AllModule ${PUBLIC_MODULE})
list(APPEND AllModule ${PRIVATE_MODULE})

foreach (module ${AllModule})
    add_subdirectory("./third/${module}")
endforeach ()

foreach (module ${PUBLIC_MODULE})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${module})
endforeach ()

foreach (module ${PRIVATE_MODULE})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${module})
endforeach ()


