#pragma once

#include <Pulsar/ObjectBase.h>
#include <CoreLib/Guid.h>
#include <CoreLib.Serialization/DataSerializer.h>

namespace pulsared
{
    class AssetSourcePackage;
}
namespace pulsar
{
    class AssetObject : public ObjectBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::AssetObject, ObjectBase);
        friend class ::pulsared::AssetSourcePackage; //editor
    public:
        virtual void SerializeBuildData(ser::Stream& stream, bool is_ser);
        sptr<AssetObject> InstantiateAsset();
    public:
        guid_t get_source_package() const { return this->package_guid_; }
        guid_t get_guid() const { return this->guid_; }
        string get_virtual_path() const { return this->virtual_path_; };
        const string& get_name() const { return this->name_; }
        void set_name(string_view name) { this->name_ = name; }
    public:
        AssetObject() = default;
        AssetObject(const AssetObject&) = delete;
        AssetObject(AssetObject&&) = delete;
        AssetObject& operator=(const AssetObject&) = delete;
    protected:
        virtual void OnInstantiateAsset(sptr<AssetObject>& obj);

    protected:
        CORELIB_REFL_DECL_FIELD(guid_);
        guid_t guid_;

        CORELIB_REFL_DECL_FIELD(package_guid_);
        guid_t package_guid_;

        CORELIB_REFL_DECL_FIELD(virtual_path_);
        string virtual_path_;

        CORELIB_REFL_DECL_FIELD(name_);
        string name_;
    };
    CORELIB_DECL_SHORTSPTR(AssetObject);


    template<typename T>
    concept baseof_assetobject = std::is_base_of<AssetObject, T>::value;

}