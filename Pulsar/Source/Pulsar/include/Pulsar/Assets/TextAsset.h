#pragma once

#include <Pulsar/AssetObject.h>

namespace pulsar
{
    class TextAsset : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::TextAsset, AssetObject);
        friend class TextAssetImporter;
    public:
        virtual void OnInstantiateAsset(AssetObject* obj) override;
    public:
        static RCPtr<TextAsset> StaticCreate(string_view str);
        static RCPtr<TextAsset> StaticCreate(string&& str);
    public:
        const string& get_text() const { return this->text_; }

    private:
        string text_;
    };
    DECL_PTR(TextAsset);

}