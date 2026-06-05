#include "Components/Component.h"

#include "CoreLib.Serialization/JsonSerializer.h"

#include <Pulsar/AssetObject.h>
#include <Pulsar/Components/Component.h>
#include <Pulsar/Components/RendererComponent.h>
#include <Pulsar/Node.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/World.h>
#include "Pulsar/Scene.h"

namespace pulsar
{

    class ComponentSerializeHook : public ser::ISerializeObjectHook
    {
    public:
        ComponentSerializeHook(SceneObjectSerializer* s) : s(s) {}

        bool IStringify_Parse(Object* object, const string& value) override
        {
            auto* boxing = dynamic_cast<BoxingSceneObjectPtrBase*>(object);
            if (!boxing)
                return false;

            // 优先用 finder 重定向（CombineFrom 时注入临时 finder）
            if (s->SceneObjectFinder)
            {
                guid_t collectionGuid;
                guid_t sceneObjId;
                BoxingSceneObjectPtrBase::Parse(value, collectionGuid, sceneObjId);

                if (auto found = s->SceneObjectFinder->FindSceneObject(sceneObjId))
                {
                    boxing->ptr = SceneObjectPtrBase::UnsafeCreate(found.GetHandle());
                    return true;
                }
            }

            // finder 中找不到，返回 false 走默认的全局资产查找
            return false;
        }

        SceneObjectSerializer* s;
    };

    void Component::Serialize(SceneObjectSerializer* s)
    {
        if (s->IsWrite)
        {
            auto json = ser::JsonSerializer::Serialize(this, {});
            s->Object->AssignParse(json);
        }
        else // read
        {
            ComponentSerializeHook hook{s};
            ser::JsonSerializer::Deserialize(s->Object->ToString(false), GetType(), self(), &hook);
        }
    }

    void Component::GetDependenciesAsset(array_list<guid_t>& deps) const
    {
        SceneObject::GetDependenciesAsset(deps);

        for (FieldInfo* field : this->GetType()->GetFieldInfos())
        {
            auto value = field->GetValue(this);
            if (!value) continue;

            // 单个 RCPtr 字段
            if (auto* rcBox = dynamic_cast<BoxingRCPtrBase*>(value.get()))
            {
                if (rcBox->ptr.IsValid())
                {
                    deps.push_back(rcBox->ptr.GetGuid());
                }
                continue;
            }

            // List / 容器字段：遍历元素检查是否是 RCPtr
            if (auto* list = dynamic_cast<IList*>(value.get()))
            {
                for (int32_t i = 0; i < list->GetCount(); ++i)
                {
                    auto item = list->At(i);
                    if (auto* itemRcBox = dynamic_cast<BoxingRCPtrBase*>(item.get()))
                    {
                        if (itemRcBox->ptr.IsValid())
                        {
                            deps.push_back(itemRcBox->ptr.GetGuid());
                        }
                    }
                }
            }
        }
    }

    ObjectPtr<Component> Component::GetMasterComponent() const
    {
        return m_masterComponent;
    }
    World* Component::GetWorld() const
    {
        return GetNode()->GetRuntimeWorld();
    }
    ObjectPtr<TransformComponent> Component::GetTransform() const
    {
        return m_ownerNode->GetTransform();
    }

    void Component::OnReceiveMessage(MessageId id)
    {

    }
    void Component::SendMessage(MessageId msgid)
    {
        OnReceiveMessage(msgid);
    }
    bool Component::EqualsComponentType(Type* type)
    {
        return this->GetType() == type;
    }

    void Component::OnTick(Ticker ticker)
    {
    }
    Component::Component()
    {
        m_flags |= OF_LifecycleManaged;
    }
    string ComponentInfoManager::GetFriendlyComponentName(Type* type)
    {
        string name = type->GetShortName();

        static char com_str[] = "Component";
        if (name.ends_with("Component"))
        {
            name = name.substr(0, name.size() - sizeof(com_str) + 1);
        }
        return StringUtil::FriendlyName(name);
    }

    NodeCollection* Component::GetOwnerNodeCollection() const
    {
        if (m_ownerNode)
            return m_ownerNode->GetOwnerNodeCollection();
        return nullptr;
    }

    void Component::BeginComponent()
    {
        m_isBegun = true;

        if (m_canDrawGizmo && !HasObjectFlags(OF_Preview))
        {
            GetWorld()->GetGizmosManager().AddGizmoComponent(self_ptr());
        }
    }
    void Component::EndComponent()
    {
        m_isBegun = false;

        if (m_canDrawGizmo && !HasObjectFlags(OF_Preview))
        {
            GetWorld()->GetGizmosManager().RemoveGizmoComponent(self_ptr());
        }
    }
} // namespace pulsar