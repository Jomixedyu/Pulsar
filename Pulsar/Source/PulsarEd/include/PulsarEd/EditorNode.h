#pragma once
#include <Pulsar/Node.h>
#include <PulsarEd/Assembly.h>

namespace pulsared
{
    class EditorNode : public Node
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::EditorNode, Node);

    public:


        static ObjectPtr<EditorNode> StaticCreate(const string& name, Node_ref parent = nullptr);
    };
    CORELIB_DECL_SHORTSPTR(EditorNode);
}