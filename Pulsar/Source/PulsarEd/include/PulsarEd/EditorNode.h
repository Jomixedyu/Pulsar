#pragma once
#include <Pulsar/Node.h>
#include <PulsarEd/Assembly.h>

namespace pulsared
{
    class EditorNode : public Node
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::EditorNode, Node);

    public:


        static sptr<EditorNode> StaticCreate(const string& name, sptr<Node> parent = nullptr);
    };
    CORELIB_DECL_SHORTSPTR(EditorNode);
}