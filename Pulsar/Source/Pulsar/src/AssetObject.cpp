#include "AssetObject.h"

namespace pulsar
{

    AssetObject_sp AssetObject::InstantiateAsset()
    {
        AssetObject_sp obj = sptr_cast<AssetObject>(this->GetType()->CreateSharedInstance({}));
        obj->Construct();
        this->OnInstantiateAsset(obj);

        return obj;
    }
    void AssetObject::OnInstantiateAsset(sptr<AssetObject>& obj)
    {
        obj->name_ = this->name_;
        assert(false);
    }
}