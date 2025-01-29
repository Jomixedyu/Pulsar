
project(imext)


add_library(${PROJECT_NAME}
        "ImGuizmo.h"
        "ImGuizmo.cpp"
        "ImGuiExt.h"
        "ImGuiExt.cpp"

        "crude_json.cpp"
        "imgui_canvas.cpp"
        "imgui_node_editor.cpp"
        "imgui_node_editor_api.cpp"

)

target_include_directories(${PROJECT_NAME} PRIVATE "../imgui/include/imgui")
target_include_directories(${PROJECT_NAME} PRIVATE "../imgui/include")
target_include_directories(${PROJECT_NAME} PUBLIC ".")
