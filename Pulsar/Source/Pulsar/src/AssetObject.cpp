#include "AssetObject.h"

namespace pulsar
{

    void AssetObject::Serialize(AssetSerializer* s)
    {
        // read or write
    }
    AssetObject::AssetObject()
    {
        new_init_sptr(m_importFiles);
        new_init_sptr(m_tags);
    }
    AssetObject_ref AssetObject::InstantiateAsset()
    {
        if (!CanInstantiateAsset())
        {
            return {};
        }
        AssetObject_sp obj = sptr_cast<AssetObject>(this->GetType()->CreateSharedInstance({}));
        obj->Construct();
        obj->SetIndexName(this->GetIndexName());

        this->OnInstantiateAsset(obj.get());

        return obj;
    }
    void AssetObject::OnInstantiateAsset(AssetObject* obj)
    {
        obj->SetIndexName(this->GetIndexName());

        ObjectFlags flag = this->GetObjectFlags() | OF_Instance;
        flag &= ~OF_Persistent;

        obj->SetObjectFlags(flag);
    }
}