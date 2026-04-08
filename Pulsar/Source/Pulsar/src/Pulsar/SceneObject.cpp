#include "SceneObject.h"
#include "AssetObject.h"
#include "Assets/NodeCollection.h"

namespace pulsar
{

    void SceneObject::SceneObjectConstruct(guid_t guid)
    {
        m_sceneObjectGuid = guid.is_empty() ? guid_t::create_new() : guid;
        Construct();
    }
    void BoxingSceneObjectPtrBase::IStringify_Parse(const string& value)
    {
       auto parts = std::views::split(value, ';') | std::ranges::to<std::vector<string>>();

       if (parts.size() != 2)
       {
           ptr = {};
           return;
       }


       auto assetId = guid_t::parse(parts[0]);
       auto sceneId = guid_t::parse(parts[1]);

       auto asset = RuntimeAssetManager::GetLoadedAssetByGuid<NodeCollection>(assetId);
       if (asset)
       {
           if (auto sceneObj = asset->FindSceneObject(sceneId))
           {
               ptr = SceneObjectPtrBase::UnsafeCreate(sceneObj.GetHandle());
           }
       }

    }

    string BoxingSceneObjectPtrBase::IStringify_Stringify()
    {
        if (auto obj = ptr.GetObjectPointer())
        {
            if (auto scene = obj->GetOwnerNodeCollection())
            {
                return StringUtil::Concat(scene->GetAssetGuid().to_string(), ";", obj->GetSceneObjectGuid().to_string());
            }
        }
        return {};
    }

} // namespace pulsar