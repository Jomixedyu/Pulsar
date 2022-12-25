#include "AssetObject.h"

namespace apatite
{
    void AssetObject::Serialize(ser::Stream& stream, bool is_ser)
    {

    }

    AssetObject_sp AssetObject::InstantiateAsset()
    {
        AssetObject_sp obj = sptr_cast<AssetObject>(this->GetType()->CreateSharedInstance({}));

        this->OnInstantiateAsset(obj);

        return obj;
    }
    void AssetObject::OnInstantiateAsset(sptr<AssetObject>& obj)
    {
        obj->name_ = this->name_;
        assert(false);
    }
}