
project(imext)


add_library(${PROJECT_NAME} "ImGuizmo.h" "ImGuizmo.cpp")

target_include_directories(${PROJECT_NAME} PRIVATE "../imgui/include/imgui")
