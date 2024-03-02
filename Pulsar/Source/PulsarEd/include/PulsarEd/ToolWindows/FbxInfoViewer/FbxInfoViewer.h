#pragma once
#include "Common/FileTreeNode.hpp"

#include <PulsarEd/Assembly.h>
#include <PulsarEd/Windows/ToolWindow.h>

namespace pulsared
{
    class FbxInfoViewer: public ToolWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::FbxInfoViewer, ToolWindow);
    public:
        FbxInfoViewer();
        virtual string_view GetWindowDisplayName() const override { return ICON_FK_WRENCH " FbxInfoViewer"; }
        virtual void OnDrawImGui(float dt);

        class FbxInfoNode : public FileTreeNode<FbxInfoNode>
        {
        public:
            array_list<string> AttributeType;
            string TypeName;
        };
        using FbxInfoNodePtr = sptr<FbxInfoNode>;

    protected:
        void ShowNode(FbxInfoNodePtr node);

    protected:
        FbxInfoNodePtr m_root;
        FbxInfoNodePtr m_selected;
        char m_fbxpath[512]{};
    };

} // namespace pulsared
