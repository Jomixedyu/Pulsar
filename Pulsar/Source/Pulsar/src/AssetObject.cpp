#include "AssetObject.h"

namespace pulsar
{

    void AssetObject::Serialize(AssetSerializer* s)
    {
        // read or write
    }
    AssetObject::AssetObject()
    {
        m_importFiles = mksptr(new List<String_sp>);
        m_tags = mksptr(new List<String_sp>);
    }
    AssetObject_ref AssetObject::InstantiateAsset()
    {
        if (!CanInstantiateAsset())
        {
            return {};
        }
        AssetObject_sp obj = sptr_cast<AssetObject>(this->GetType()->CreateSharedInstance({}));
        obj->Construct();

        this->OnInstantiateAsset(obj.get());

        return obj;
    }
    void AssetObject::OnInstantiateAsset(AssetObject* obj)
    {
        obj->m_name = this->m_name;

        ObjectFlags flag = this->GetObjectFlags() | OF_Instance;
        flag &= ~OF_Persistent;

        obj->SetObjectFlags(flag);
    }
}