#pragma once
#include <Apatite/Node.h>
#include <ApatiteEd/Assembly.h>

namespace apatiteed
{
    class EditorNode : public Node
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::EditorNode, Node);

    public:


    };
    CORELIB_DECL_SHORTSPTR(EditorNode);
}