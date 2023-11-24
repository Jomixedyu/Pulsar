#pragma once

#include <Pulsar/ObjectBase.h>
#include <CoreLib/Guid.h>
#include <CoreLib/sser.hpp>
#include <CoreLib.Serialization/DataSerializer.h>
#include <CoreLib.Serialization/ObjectSerializer.h>
#include <iostream>
#include <utility>

namespace pulsar
{

    struct AssetSerializer
    {
        AssetSerializer(ser::VarientRef obj, std::iostream& stream, bool isWrite, bool editorData)
            : Object(std::move(obj)),
              Stream(stream),
              IsWrite(isWrite),
              HasEditorData(editorData),
              ExistStream(false)
        {
        }

        AssetSerializer(const AssetSerializer&) = delete;
        AssetSerializer(AssetSerializer&&) = delete;

    public:
        ser::VarientRef Object;
        std::iostream& Stream;
        bool ExistStream;
        const bool IsWrite;
        const bool HasEditorData;
    };

    class AssetObject : public ObjectBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::AssetObject, ObjectBase);

    public:
        virtual void Serialize(AssetSerializer* s);
        virtual bool CanInstantiateAsset() const { return true; }
        ObjectPtr<AssetObject> InstantiateAsset();

    public:
    public:
        AssetObject();
        AssetObject(const AssetObject&) = delete;
        AssetObject(AssetObject&&) = delete;
        AssetObject& operator=(const AssetObject&) = delete;

    protected:
        virtual void OnInstantiateAsset(AssetObject* obj);

    protected:
        #ifdef WITH_EDITOR

        CORELIB_REFL_DECL_FIELD(m_importFiles);
        List_sp<String_sp> m_importFiles;

        array_list<string> m_extraEditorMetadata;

        #endif

        CORELIB_REFL_DECL_FIELD(m_tags);
        List_sp<String_sp> m_tags;
    };

    DECL_PTR(AssetObject);


    template <typename T>
    concept baseof_assetobject = std::is_base_of_v<AssetObject, T>;

}
