#include "EditorNode.h"


namespace pulsared
{
    ObjectPtr<EditorNode> EditorNode::StaticCreate(const string& name, Node_ref parent)
    {
        sptr<EditorNode> node = mksptr(new EditorNode);
        node->Construct();
        node->SetName(name);
        node->SetParent(parent);
        return node;
    }
}