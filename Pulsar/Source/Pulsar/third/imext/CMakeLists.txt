
project(imext)


add_library(${PROJECT_NAME}
        "ImGuizmo.h"
        "ImGuizmo.cpp"
        "ImCurveEdit.h"
        "ImCurveEdit.cpp"
        "ImGradient.h"
        "ImGradient.cpp"

        "ImGuiExt.h"
        "ImGuiExt.cpp"

        "crude_json.h"
        "crude_json.cpp"
        "imgui_bezier_math.h"
        "imgui_bezier_math.inl"
        "imgui_canvas.cpp"
        "imgui_canvas.h"
        "imgui_extra_math.h"
        "imgui_extra_math.inl"
        "imgui_node_editor.cpp"
        "imgui_node_editor.h"
        "imgui_node_editor_api.cpp"
        "imgui_node_editor_internal.h"
        "imgui_node_editor_internal.inl"

        "utilities/builders.h"
        "utilities/builders.cpp"
        "utilities/drawing.h"
        "utilities/drawing.cpp"
        "utilities/widgets.h"
        "utilities/widgets.cpp"

)

target_include_directories(${PROJECT_NAME} PRIVATE "../imgui/include/imgui")
target_include_directories(${PROJECT_NAME} PRIVATE "../imgui/include")
target_include_directories(${PROJECT_NAME} PUBLIC ".")
