#pragma once
#include "Assembly.h"
#include <Pulsar/AssetObject.h>
#include <filesystem>
#include "Workspace.h"

CORELIB_DECL_LIST(pulsared::string);

namespace pulsared
{
    class AssetMetaData : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::AssetMetaData, Object);

    public:
        CORELIB_REFL_DECL_FIELD(Type);

        string Type;
        CORELIB_REFL_DECL_FIELD(Handle);

        ObjectHandle Handle;
        CORELIB_REFL_DECL_FIELD(ExtraFiles);

        List_sp<string> ExtraFiles;
        CORELIB_REFL_DECL_FIELD(ImportFiles);
        List_sp<string> ImportFiles;
    };

    CORELIB_DECL_SHORTSPTR(AssetMetaData);

    struct AssetFileNode : public std::enable_shared_from_this<AssetFileNode>
    {
    public:
        std::filesystem::path PhysicsPath;
        string AssetName;
        string AssetPath;
        bool IsFolder;
        bool IsPhysicsFile;

        bool Valid;
        AssetMetaData_sp AssetMeta;
        std::weak_ptr<AssetFileNode> Parent;
        array_list<std::shared_ptr<AssetFileNode>> Children;

        bool IsSelected = false;
        bool IsCollapsed = false;

        string GetPhysicsPath() const;
        string GetPhysicsName() const;
        string GetPhysicsNameWithoutExt() const;
        string GetPhysicsNameExt() const;
        string GetPackageName() const;

        Type* GetAssetType() const;

        std::shared_ptr<AssetFileNode> GetChild(string_view name);
        std::shared_ptr<AssetFileNode> Find(string_view path);
    };

    class FolderAsset : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::FolderAsset, Object);

    public:
    };


    class AssetIconPool final
    {
    public:
        gfx::GFXDescriptorSet_wp GetDescriptorSet(const index_string& id);
        gfx::GFXDescriptorSet_wp GetDescriptorSet(Type* id);
        void Register(const index_string& id, const uint8_t* iconBuf, size_t length);
        void ClearCache();
        AssetIconPool();
        ~AssetIconPool();

    private:
        hash_map<index_string, gfx::GFXDescriptorSet_sp> m_cacheDescSets;
        hash_map<index_string, gfx::GFXTexture2D_sp> m_textures;
        gfx::GFXDescriptorSetLayout_sp m_descriptorLayout;
    };

    class AssetDatabase
    {
    public:
        static void Initialize();
        static void Refresh();
        static void Terminate();

        static void AddProgramPackageSearchPath(std::filesystem::path path);
        static void AddPackage(string_view packageName);
        static std::filesystem::path GetPackagePath(string_view packageName);
        static std::filesystem::path GetAbsoluteAssetPath(string_view assetPath);
        static array_list<ProgramPackage> GetPackageInfos();

        static AssetObject_ref LoadAssetAtPath(string_view path);
        static AssetObject_ref LoadAssetById(ObjectHandle id);
        static string GetPathByAsset(AssetObject_ref asset);
        static ObjectHandle GetIdByPath(string_view path);

        static array_list<string> GetFoldersByPath(string_view path);

        static bool ExistAsset(AssetObject_ref asset);
        static void ReloadAsset(ObjectHandle id);
        static void Save(AssetObject_ref asset);
        static void SaveAll();
        static void NewAsset(string_view path, Type* assetType);
        static bool CreateAsset(AssetObject_ref asset, string_view path);
        static bool DeleteAsset(AssetObject_ref asset);

        static void MarkDirty(AssetObject_ref asset);
        static bool IsDirty(AssetObject_ref asset);
        static void ResolveDirty(AssetObject_ref asset);

        static inline const char* FileTreeRootPath = "Packages";

        static inline Function<bool, AssetObject_ref> OnRequestDeleteAsset;
        static inline Action<AssetObject_ref> OnDeletedAsset;

        static inline Function<bool, AssetObject_ref> OnRequestSaveAsset;
        static inline Action<AssetObject_ref> OnSavedAsset;

        static inline Function<bool, AssetObject_ref, string_view> OnRequestCreateAsset;
        static inline Action<AssetObject_ref> OnCreatedAsset;

        static inline std::shared_ptr<AssetFileNode> FileTree;
        static inline std::unique_ptr<AssetIconPool> IconPool;

    protected:
        static void OnAddPackage(ProgramPackage* package);
    };

    inline std::filesystem::path operator"" _adpath(const char* str, size_t len)
    {
        return AssetDatabase::GetAbsoluteAssetPath(str);
    }

    inline string operator"" _adspath(const char* str, size_t len)
    {
        return AssetDatabase::GetAbsoluteAssetPath(str).string();
    }

}
