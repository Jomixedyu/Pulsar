#pragma once
#include <PulsarEd/Windows/ToolWindow.h>


namespace pulsared
{
    class ShaderDebugTool : public ToolWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ShaderDebugTool, ToolWindow);
    public:
        ShaderDebugTool();
        virtual string_view GetWindowDisplayName() const override { return ICON_FK_WRENCH " ShaderDebugTool"; }
        virtual void OnDrawImGui(float dt);

        void RefreshAssets();
        void OnOpen() override;
    private:
        array_list<RCPtr<Shader>> m_shaders;
        array_list<string> m_shaderPaths;
    };
}