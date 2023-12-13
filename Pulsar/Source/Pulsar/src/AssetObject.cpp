#include "AssetObject.h"

namespace pulsar
{

    template <typename T>
    void new_init_sptr(sptr<T>& ptr)
    {
        if (!ptr)
        {
           ptr = mksptr(new T);
        }
    }

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