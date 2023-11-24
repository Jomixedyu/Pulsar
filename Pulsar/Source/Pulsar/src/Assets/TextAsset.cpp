#include "Assets/TextAsset.h"
namespace pulsar
{

    void TextAsset::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
        TextAsset* i = static_cast<ThisClass*>(obj);
        i->text_ = this->text_;
    }

    ObjectPtr<TextAsset> TextAsset::StaticCreate(const string& str)
    {
        TextAsset* txt = new TextAsset;
        txt->text_ = str;
        return mksptr(txt);
    }

    ObjectPtr<TextAsset> TextAsset::StaticCreate(string&& str)
    {
        TextAsset* txt = new TextAsset;
        txt->text_ = std::move(str);
        return mksptr(txt);
    }


}