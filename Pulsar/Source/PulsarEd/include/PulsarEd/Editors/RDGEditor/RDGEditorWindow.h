#pragma once
#include "Editors/AssetEditor/AssetPreviewEditorWindow.h"
#include <Pulsar/Rendering/RDG/RDGGraph.h>

namespace pulsared
{
    class RDGEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::RDGEditorWindow, AssetEditorWindow);
        DEFINE_ASSET_EDITOR(RDGGraph, false);
    public:
        virtual string_view GetWindowDisplayName() const { return "RenderGraph Editor"; }

    protected:
        virtual void OnDrawImGui(float dt) override;
        virtual void OnOpen() override;
        virtual void OnClose() override;
        void OnRefreshMenuContexts() override;

        RCPtr<RDGGraph> m_graph;

        void* m_externalContext = nullptr;
    };
}