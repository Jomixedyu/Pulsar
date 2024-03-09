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
    class MenuItemCreateAssetAttribute : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::MenuItemCreateAssetAttribute, Attribute);
    public:

    };

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
        OSPlatform Platform;
        bool CookedOnly;
        bool ExistStream;
        const bool IsWrite;
        const bool HasEditorData;
    };

    class ImportedFileInfo : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ImportedFileInfo, Object);
    public:
        CORELIB_REFL_DECL_FIELD(m_filename);
        String_sp m_filename;
        CORELIB_REFL_DECL_FIELD(m_latestModification);
        String_sp m_latestModification;
        CORELIB_REFL_DECL_FIELD(m_hash);
        String_sp m_hash;

        ImportedFileInfo()
        {
            init_sptr_member(m_filename);
            init_sptr_member(m_latestModification);
            init_sptr_member(m_hash);
        }
    };
    CORELIB_DECL_SHORTSPTR(ImportedFileInfo);

    class AssetObject : public ObjectBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::AssetObject, ObjectBase);

    public:
        virtual void Serialize(AssetSerializer* s);
        virtual bool CanInstantiateAsset() const { return true; }
        RCPtr<AssetObject> InstantiateAsset();

    public:
        AssetObject();
        AssetObject(const AssetObject&) = delete;
        AssetObject(AssetObject&&) = delete;
        AssetObject& operator=(const AssetObject&) = delete;

        void Incref() { ++m_cref; }
        void Decref();
        uint32_t GetCref() const { return m_cref; }
    protected:
        virtual void OnInstantiateAsset(AssetObject* obj);

    protected:
        #ifdef WITH_EDITOR

        CORELIB_REFL_DECL_FIELD(m_importFiles);
        List_sp<ImportedFileInfo_sp> m_importFiles;

        array_list<string> m_extraEditorMetadata;

        #endif

        CORELIB_REFL_DECL_FIELD(m_tags);
        List_sp<String_sp> m_tags;

        uint32_t m_cref{};
    };

    DECL_PTR(AssetObject);


    template <typename T>
    concept baseof_assetobject = std::is_base_of_v<AssetObject, T>;

}
