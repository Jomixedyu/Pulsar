#include "AssetObject.h"

namespace pulsar
{

    AssetObject_sp AssetObject::InstantiateAsset()
    {
        AssetObject_sp obj = sptr_cast<AssetObject>(this->GetType()->CreateSharedInstance({}));
        obj->Construct();

        ObjectPtr<AssetObject> mobj = obj;

        this->OnInstantiateAsset(mobj);

        return obj;
    }
    void AssetObject::OnInstantiateAsset(ObjectPtr<AssetObject>& obj)
    {
        obj->name_ = this->name_;
        assert(false);
    }
}