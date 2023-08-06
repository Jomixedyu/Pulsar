#pragma once

#include <Pulsar/AssetObject.h>

namespace pulsar
{
    class TextAsset : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::TextAsset, AssetObject);
        friend class TextAssetImporter;
    public:
        virtual void OnInstantiateAsset(sptr<AssetObject>& obj) override;
    public:
        static sptr<TextAsset> StaticCreate(const string& str);
        static sptr<TextAsset> StaticCreate(string&& str);
    public:
        const string& get_text() const { return this->text_; }

    private:
        string text_;
    };
    DECL_PTR(TextAsset);

}