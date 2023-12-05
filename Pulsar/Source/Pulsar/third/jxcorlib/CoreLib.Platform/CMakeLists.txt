
cmake_minimum_required(VERSION 3.8)

project(jxcorlib.platform)


list(APPEND LIST_SRC
        "Window.h"
        "FolderWatch.h"
        "System.h"
)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    list(APPEND LIST_SRC
            "Impl/Windows/Window.cpp"
            "Impl/Windows/FolderWatch.cpp"
            "Impl/Windows/System.cpp"
    )
endif ()


add_library(${PROJECT_NAME} OBJECT ${LIST_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE "./")
