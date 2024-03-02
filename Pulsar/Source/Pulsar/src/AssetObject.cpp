#include "AssetObject.h"

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

    void AssetObject::Decref()
    {
        assert(m_cref > 0);
        --m_cref;
        if (m_cref == 0)
        {
            DestroyObject(THIS_REF);
        }
    }

    void AssetObject::OnInstantiateAsset(AssetObject* obj)
    {
        obj->SetIndexName(this->GetIndexName());

        ObjectFlags flag = this->GetObjectFlags() | OF_Instance;
        flag &= ~OF_Persistent;

        obj->SetObjectFlags(flag);
    }
}