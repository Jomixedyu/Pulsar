#include "Assets/TextAsset.h"
namespace pulsar
{

    void TextAsset::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
        TextAsset* i = static_cast<ThisClass*>(obj);
        i->text_ = this->text_;
    }

    RCPtr<TextAsset> TextAsset::StaticCreate(string_view str)
    {
        auto self = mksptr(new TextAsset);
        self->Construct();
        self->text_ = str;
        return self.get();
    }

    RCPtr<TextAsset> TextAsset::StaticCreate(string&& str)
    {
        auto self = mksptr(new TextAsset);
        self->Construct();
        self->text_ = std::move(str);
        return self.get();
    }


}