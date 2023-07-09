#include "TextAsset.h"
#include "TextAsset.h"

namespace pulsar
{

    void TextAsset::SerializeBuildData(ser::Stream& stream, bool is_ser)
    {
        ser::ReadWriteStream(stream, is_ser, this->text_);
    }

    void TextAsset::OnInstantiateAsset(sptr<AssetObject>& obj)
    {
        auto i = sptr_cast<TextAsset>(obj);
        i->text_ = this->text_;
    }

    sptr<TextAsset> TextAsset::StaticCreate(const string& str)
    {
        TextAsset* txt = new TextAsset;
        txt->text_ = str;
        return mksptr(txt);
    }

    sptr<TextAsset> TextAsset::StaticCreate(string&& str)
    {
        TextAsset* txt = new TextAsset;
        txt->text_ = std::move(str);
        return mksptr(txt);
    }


}