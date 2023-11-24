
cmake_minimum_required (VERSION 3.8)

project (jxcorlib.platform)


list(APPEND LIST_SRC "Window.h")
list(APPEND LIST_SRC "FolderWatch.h")


if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    list(APPEND LIST_SRC "Impl/Windows/Window.cpp")
    list(APPEND LIST_SRC "Impl/Windows/FolderWatch.cpp")
endif ()


add_library(${PROJECT_NAME} STATIC OBJECT ${LIST_SRC})
