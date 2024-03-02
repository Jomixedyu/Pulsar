#pragma once
#include "Assembly.h"
#include "Common/FileTreeNode.hpp"
#include "Common/PersistentImagePool.h"
#include "Workspace.h"
#include <Pulsar/AssetObject.h>
#include <filesystem>

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

        CORELIB_REFL_DECL_FIELD(Tags);
        List_sp<string> Tags;
    };

    CORELIB_DECL_SHORTSPTR(AssetMetaData);

    struct AssetFileNode : public FileTreeNode<AssetFileNode>
    {
    public:
        AssetMetaData_sp AssetMeta;
        bool IsSelected = false;
        bool IsCollapsed = false;

        string GetPackageName() const { return GetRootName(); }
        Type* GetAssetType() const;
    };
    using AssetFileNodePtr = std::shared_ptr<AssetFileNode>;

    class FolderAsset : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::FolderAsset, Object);

    public:
    };

    // PackagePath : Engine/Layout/xxx  (Engine/Layout/xxx)
    // AssetPath   : Engine/Material/xxx (Engine/AssetsMaterial/xxx)
    // PhysicsPath

    struct AssetFilter
    {
        array_list<Type*> WhiteList;
        array_list<Type*> BlackList;
        string FolderPath;
    };
    class AssetDatabase
    {
    public:
        static void Initialize();
        static void Refresh();
        static void Terminate();

        static void AddProgramPackageSearchPath(const std::filesystem::path& path);
        static void AddPackage(string_view packageName);
        static std::filesystem::path GetPackagePhysicsPath(string_view packageName);
        static string GetPackageName(string_view assetPath);

        static std::filesystem::path PackagePathToPhysicsPath(string_view packagePath);
        static string AssetPathToPackagePath(string_view assetPath);
        static std::filesystem::path AssetPathToPhysicsPath(string_view assetPath);
        static string AssetPathToAssetName(string_view assetPath);
        static string AssetPathToParentPath(string_view path);
        static bool IsFolderPath(string_view assetPath);

        static array_list<ProgramPackage> GetPackageInfos();

        static AssetObject_ref LoadAssetAtPath(string_view path);
        static AssetObject_ref LoadAssetById(ObjectHandle id);
        static bool ExistsAssetPath(string_view path);
        static string GetPathById(ObjectHandle id);
        static string GetPathByAsset(AssetObject_ref asset);
        static ObjectHandle GetIdByPath(string_view path);
        static string GetUniquePath(string_view path);

        static bool ExistsAsset(AssetObject_ref asset);
        static void ReloadAsset(ObjectHandle id);
        static void Save(AssetObject_ref asset);
        static void SaveAll();
        static void NewAsset(string_view folderPath, string_view assetName, Type* assetType);
        static bool CreateAsset(AssetObject_ref asset, string_view path);
        // Delete assets without folder
        static bool DeleteAssets(const array_list<string>& assetPaths, array_list<string>* errinfo = nullptr);
        // Only empty folders can be deleted
        static void DeleteEmptyFolder(string_view path);
        static void DeleteFolder(string_view path, array_list<string>* errinfo = nullptr);

        static array_list<string> FindAssets(const AssetFilter& filter);
        static array_list<string> FindAssets(Type* type, string_view folderPath = {});
        static bool IsEmptyFolder(string_view path);

        static bool Rename(string_view srcAsset, string_view dstAsset);

        static void MarkDirty(AssetObject_ref asset);
        static bool IsDirty(AssetObject_ref asset);
        static void ResolveDirty(AssetObject_ref asset);


        static inline const char* FileTreeRootPath = "Packages";

        static inline Function<bool, string_view, array_list<string>*> OnRequestDeleteAsset;
        static inline Action<string_view> OnDeletedAsset;

        static inline Function<bool, AssetObject_ref> OnRequestSaveAsset;
        static inline Action<AssetObject_ref> OnSavedAsset;

        static inline Function<bool, AssetObject_ref, string_view> OnRequestCreateAsset;
        static inline Action<AssetObject_ref> OnCreatedAsset;

        static inline std::shared_ptr<AssetFileNode> FileTree;
        static inline std::unique_ptr<PersistentImagePool> IconPool;

    protected:
        static void OnAddPackage(ProgramPackage* package);
    };


}
