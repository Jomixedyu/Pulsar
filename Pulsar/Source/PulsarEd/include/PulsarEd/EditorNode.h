#pragma once
#include <Pulsar/Node.h>
#include <PulsarEd/Assembly.h>

namespace pulsared
{
    class [[deprecated]] EditorNode : public Node
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::EditorNode, Node);

    public:

    } ;
    CORELIB_DECL_SHORTSPTR(EditorNode);
}