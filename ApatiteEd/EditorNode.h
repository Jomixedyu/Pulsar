#pragma once
#include <Apatite/Node.h>
#include <ApatiteEd/Assembly.h>

namespace apatiteed
{
    class EditorNode : public Node
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::EditorNode, Node);

    public:


        static sptr<EditorNode> StaticCreate(const string& name, sptr<Node> parent = nullptr);
    };
    CORELIB_DECL_SHORTSPTR(EditorNode);
}