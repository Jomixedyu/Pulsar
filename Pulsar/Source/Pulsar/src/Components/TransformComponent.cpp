#include "Components/TransformComponent.h"
#include <Pulsar/Node.h>

namespace pulsar
{
    ObjectPtr<TransformComponent> TransformComponent::FindByName(string_view name) const
    {
        for (auto& comp : *m_children)
        {
            if (IsValid(comp))
            {
                if (comp->GetAttachedNode()->GetName() == name)
                {
                    return comp;
                }
            }
        }
        return nullptr;
    }

    ObjectPtr<TransformComponent> TransformComponent::FindByPath(string_view path) const
    {
        array_list<string> paths = StringUtil::Split(path, u8char('/'));

        TransformComponent_ref target = this->GetObjectHandle();

        for (auto& comp : *m_children)
        {
            auto targetComp = comp;
            bool success = true;
            for (size_t i = 0; i < paths.size(); i++)
            {
                if (auto find = target->FindByName(paths[i]))
                {
                    targetComp = find;
                }
                else
                {
                    success = false;
                    break;
                }
            }
            if (!success)
            {
                break;
            }
            return targetComp;
        }

        return nullptr;
    }
    TransformComponent::TransformComponent()
    {
        m_flags |= OF_DontDestroy;
        m_children = mksptr(new List<ObjectPtr<TransformComponent>>);
    }

    void TransformComponent::SetParent(ObjectPtr<TransformComponent> parent)
    {
        if (m_parent == parent)
        {
            return;
        }

        if (parent == nullptr)
        {
            // set empty
            auto it = std::find(m_parent->m_children->begin(), m_parent->m_children->end(), THIS_REF);
            m_parent->m_children->erase(it);
            m_parent = nullptr;
        }
        else
        {
            m_parent = parent;
            m_parent->m_children->push_back(THIS_REF);
        }

    }
}