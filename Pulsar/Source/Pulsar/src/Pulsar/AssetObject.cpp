#include "AssetObject.h"
#include <sstream>

namespace pulsar
{

    void AssetObject::Serialize(AssetSerializer* s)
    {
        // read or write
    }
    AssetObject::AssetObject()
    {
        init_sptr_member(m_importFiles);
        init_sptr_member(m_tags);
    }

    void AssetObject::OnInstantiateAsset(AssetObject* obj)
    {
        obj->SetIndexName(this->GetIndexName());
    }
    void AssetObject::AssetConstruct(guid_t guid)
    {
        m_assetGuid = guid;
        if (!m_assetGuid)
        {
            m_assetGuid = guid_t::create_new();
        }
        Construct();
        RuntimeAssetManager::Register(this);
    }

    void AssetObject::Destroy()
    {
        RuntimeAssetManager::Unregister(this);
        base::Destroy();
    }


    static auto& _AssetGuidToHandle()
    {
        static std::unordered_map<guid_t, ObjectHandle> _AssetGuidToHandle;
        return _AssetGuidToHandle;
    }
    ObjectHandle RuntimeAssetManager::GetLoadedAssetHandleByGuid(guid_t id)
    {
        if (id)
        {
            auto& map = _AssetGuidToHandle();
            auto it = map.find(id);
            if (it != map.end())
            {
                return it->second;
            }
        }
        return {};
    }
    RCPtrBase RuntimeAssetManager::GetLoadedAssetByGuid(guid_t id)
    {
        RCPtrBase ptr = RCPtrBase::UnsafeCreate(GetLoadedAssetHandleByGuid(id));
        return ptr;
    }


    void RuntimeAssetManager::Register(AssetObject* obj)
    {
        if (obj)
        {
            if (auto guid = obj->GetAssetGuid())
            {
                auto& map = _AssetGuidToHandle();
                map[guid] = obj->GetObjectHandle();
            }
        }

    }
    void RuntimeAssetManager::Unregister(AssetObject* obj)
    {
        if (obj)
        {
            if (auto guid = obj->GetAssetGuid())
            {
                auto& map = _AssetGuidToHandle();
                map.erase(guid);
            }
        }
    }

    bool RuntimeAssetManager::IsLoaded(guid_t guid)
    {
        auto& map = _AssetGuidToHandle();
        return map.contains(guid);
    }


    RCPtrBase InstantiateAsset(const RCPtrBase& asset)
    {
        if (!asset)
        {
            return nullptr;
        }
        if (!asset->CanInstantiateAsset())
        {
            return {};
        }

        auto obj = ser::CreateVarient("json");

        std::stringstream ss;

        AssetSerializer write { obj, ss, true, true};
        asset->Serialize(&write);

        AssetSerializer read { obj, ss, false, true};
        auto newAsset = NewAssetObject(asset->GetType());
        newAsset->Serialize(&read);

        return newAsset;
    }

    void BoxingRCPtrBase::IStringify_Parse(const string& value)
    {
        guid_t guid = guid_t::parse(value);
        ptr = RuntimeAssetManager::GetLoadedAssetByGuid(guid);
    }

    string BoxingRCPtrBase::IStringify_Stringify()
    {
        guid_t guid;
        if (auto obj = ptr.GetAssetPointer())
        {
            guid = obj->GetAssetGuid();
        }
        return guid.to_string();
    }



    RCPtrBase InternalNewAssetObject(Type* type, guid_t assetGuid)
    {
        auto obj = sptr_cast<AssetObject>(type->CreateSharedInstance({}));
        if (obj)
        {
            obj->AssetConstruct(assetGuid);
            RCPtrBase rc = RCPtrBase::UnsafeCreate(obj->GetObjectHandle());
            return rc;
        }
        return nullptr;
    }

}