#include "EditorNode.h"


namespace pulsared
{
    sptr<EditorNode> EditorNode::StaticCreate(const string& name, sptr<Node> parent)
    {
        sptr<EditorNode> node = mksptr(new EditorNode);
        node->Construct();
        node->SetName(name);
        node->set_parent(parent);
        return node;
    }
}