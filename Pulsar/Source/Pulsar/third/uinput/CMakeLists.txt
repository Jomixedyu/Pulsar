cmake_minimum_required (VERSION 3.23)

project ("uinput")

set(CMAKE_CXX_STANDARD 23)

set(UINPUT_SRC_LIST
        "./src/uinput/InputKeyCode.cpp"
        "./src/uinput/InputManager.cpp"
        "./src/uinput/InputManagerSDL.cpp"
)

add_library(uinput ${UINPUT_SRC_LIST})


get_target_property(target_type uinput TYPE)
if(target_type STREQUAL "SHARED_LIBRARY")
    target_compile_definitions(${PROJECT_NAME} PUBLIC UINPUT_BUILD_SHARED)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE "../SDL2lib/SDL/include")

target_include_directories(${PROJECT_NAME} PUBLIC "./include")
target_include_directories(${PROJECT_NAME} PRIVATE "./include/uinput")
target_compile_definitions(${PROJECT_NAME} PRIVATE UINPUT_EXPORT_API)


