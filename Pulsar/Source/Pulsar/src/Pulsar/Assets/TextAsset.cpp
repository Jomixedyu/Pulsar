#include "Assets/TextAsset.h"
namespace pulsar
{

    RCPtr<TextAsset> TextAsset::StaticCreate(string_view str)
    {
        auto self = NewAssetObject<TextAsset>();
        self->text_ = str;
        return self;
    }

    RCPtr<TextAsset> TextAsset::StaticCreate(string&& str)
    {
        auto self = NewAssetObject<TextAsset>();
        self->text_ = std::move(str);
        return self;
    }


}