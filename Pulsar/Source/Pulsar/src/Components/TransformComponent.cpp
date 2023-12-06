#include "Components/TransformComponent.h"

#include "TransformUtil.h"

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

    Vector3f TransformComponent::GetWorldPosition() const
    {
        return GetLocalToWorldMatrix() * m_position;
    }
    void TransformComponent::SetWorldPosition(Vector3f value)
    {
        m_position = GetWorldToLocalMatrix() * value;
        GetAttachedNode()->SendMessage(MessageId_OnChangedTransform);
    }
    void TransformComponent::Translate(Vector3f value)
    {
        SetPosition(GetPosition() + value);
    }

    Vector3f TransformComponent::GetWorldScale() const
    {
        return GetLocalToWorldMatrix() * m_scale;
    }

    Vector3f TransformComponent::GetEuler() const
    {
        return m_rotation.GetEuler();
    }

    void TransformComponent::SetEuler(Vector3f value)
    {
        m_rotation = Quat4f::FromEuler(value);
        BroadcastChange();
    }

    TransformComponent::TransformComponent()
        : m_localToWorldMatrix{1.f}, m_worldToLocalMatrix{1.f},
        m_scale(1.f,1.f,1.f)
    {
        m_flags |= OF_DontDestroy;
        m_children = mksptr(new List<ObjectPtr<TransformComponent>>);
    }
    void TransformComponent::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);
        RebuildLocalToWorldMatrix();
    }

    Matrix4f TransformComponent::GetChildLocalToWorldMatrix() const
    {
        Matrix4f selfMat{1};
        transutil::NewTRS(selfMat, m_position, m_rotation, m_scale);
        return m_localToWorldMatrix * selfMat;
    }

    void TransformComponent::RebuildLocalToWorldMatrix()
    {
        if (m_parent)
        {
            m_localToWorldMatrix = m_parent->GetChildLocalToWorldMatrix();
        }
    }
    void TransformComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == NAMEOF(m_euler))
        {
            m_rotation = Quat4f::FromEuler(m_euler);
            BroadcastChange();
        }
    }
    void TransformComponent::BroadcastChange()
    {
        GetAttachedNode()->SendMessage(MessageId_OnChangedTransform);
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