#include "AssetDatabase.h"
#include "AssetProviders/AssetProvider.h"
#include "Workspace.h"
#include <CoreLib.Serialization/JsonSerializer.h>
#include <CoreLib/File.h>
#include <PulsarEd/AssetProviders/AssetProvider.h>
#include <filesystem>
#include <fstream>
#include <gfx/GFXImage.h>
#include <ranges>
#include <unordered_set>

namespace pulsared
{

    static std::unordered_set<ObjectHandle> _DirtyObjects;

    class PackageAssetRegistry
    {
    public:
        hash_map<ObjectHandle, string> AssetPathMapping;
    };

    static hash_map<string, PackageAssetRegistry> _AssetRegistry;

    static void _Scan(std::shared_ptr<AssetFileNode> node, const std::function<void(std::shared_ptr<AssetFileNode>)>& proc)
    {
        for (auto& i : std::filesystem::directory_iterator(node->PhysicsPath))
        {
            std::shared_ptr<AssetFileNode> newNode = mksptr(new AssetFileNode);

            newNode->IsFolder = i.is_directory();
            newNode->PhysicsPath = i.path();
            newNode->AssetName = i.path().stem().string();
            newNode->AssetPath = node->AssetPath + '/' + newNode->AssetName;

            if (newNode->GetPhysicsNameExt() == ".pmeta")
            {
                newNode->AssetMeta = ser::JsonSerializer::Deserialize<AssetMetaData>(FileUtil::ReadAllText(newNode->PhysicsPath.string()));
            }
            else if (!newNode->IsFolder)
            {
                continue;
            }

            proc(newNode);

            if (newNode->IsFolder)
            {
                _Scan(newNode, proc);
            }
            node->Children.push_back(newNode);
        }
        node->Sort();
    }

    static void _OnWorkspaceOpened()
    {

    }

    AssetObject_ref AssetDatabase::LoadAssetAtPath(string_view path)
    {
        string pathStr{path};
        //path = pathStr.insert(pathStr.find('/'), "/Assets");

        auto node = FileTree->Find(path).get();
        if (!node)
        {
            return nullptr;
        }

        sptr<AssetObject> assetObj;

        auto json = FileUtil::ReadAllText(node->GetPhysicsPath());
        auto meta = ser::JsonSerializer::Deserialize<AssetMetaData>(json);

        if (auto existObj = RuntimeObjectWrapper::GetObject(meta->Handle))
        {
            return static_cast<AssetObject*>(existObj);
        }

        auto type = AssemblyManager::GlobalFindType(meta->Type);
        if(!type)
        {
            Logger::Log("not found type.", LogLevel::Error);
            return nullptr;
        }
        assetObj = sptr_cast<AssetObject>(type->CreateSharedInstance({}));

        assetObj->SetName(node->AssetName);
        assetObj->Construct(meta->Handle);

        {
            auto assetPath = node->PhysicsPath;
            assetPath.replace_extension({".pa"});

            auto assetBinPath = node->PhysicsPath;
            assetBinPath.replace_extension({".pba"});

            auto fileJson = FileUtil::ReadAllText(assetPath.string());
            auto objser = ser::CreateVarient("json");
            objser->AssignParse(fileJson);

            auto fs = std::fstream{assetBinPath, std::ios::in | std::ios::out | std::ios::binary};

            auto serializer = AssetSerializer{objser, fs, false, true};
            serializer.ExistStream = fs.is_open();
            assetObj->Serialize(&serializer);
        }

        return assetObj;
    }

    AssetObject_ref AssetDatabase::LoadAssetById(ObjectHandle id)
    {
        for (auto& [packageName, registry] : _AssetRegistry)
        {
            auto it = registry.AssetPathMapping.find(id);
            if (it != registry.AssetPathMapping.end())
            {
                return LoadAssetAtPath(it->second);
            }
        }
        return nullptr;
    }

    string AssetDatabase::GetPathByAsset(AssetObject_ref asset)
    {
        for (auto& [package, registry] : _AssetRegistry)
        {
            auto it = registry.AssetPathMapping.find(asset.GetHandle());
            if (it != registry.AssetPathMapping.end())
            {
                return it->second;
            }
        }

        return {};
    }

    ObjectHandle AssetDatabase::GetIdByPath(string_view path)
    {
        for (auto& [package, registry] : _AssetRegistry)
        {
            for (auto& [k, v] : registry.AssetPathMapping)
            {
                if (v == path)
                    return k;
            }
        }
        return {};
    }

    array_list<string> AssetDatabase::GetFoldersByPath(string_view path)
    {
        return {};
    }


    void AssetDatabase::ResolveDirty(AssetObject_ref asset)
    {
        auto it = _DirtyObjects.find(asset.GetHandle());
        if (it != _DirtyObjects.end())
        {
            _DirtyObjects.erase(it);
        }
    }

    bool AssetDatabase::ExistAsset(AssetObject_ref asset)
    {
        return !GetPathByAsset(asset).empty();
    }

    void AssetDatabase::ReloadAsset(ObjectHandle id)
    {
        RuntimeObjectWrapper::DestroyObject(id, true);
        LoadAssetById(id);
        ResolveDirty(id);
    }

    void AssetDatabase::Save(AssetObject_ref asset)
    {
        if (!asset)
            return;
        if (!IsDirty(asset))
        {
            return;
        }
        bool result = OnRequestSaveAsset.IsValidReturnInvoke(asset);

        if (result)
        {
            auto assetPath = GetPathByAsset(asset);
            auto node = FileTree->Find(assetPath);
            auto assetPhyicsPath = node->PhysicsPath;
            assetPhyicsPath.replace_extension({".pa"});

            auto assetBinPath = node->PhysicsPath;
            assetBinPath.replace_extension({".pba"});

            auto fs = std::fstream{assetBinPath, std::ios::out | std::ios::trunc | std::ios::binary};

            const ser::VarientRef textAssetObject = ser::CreateVarient("json");

            auto ser = AssetSerializer{textAssetObject, fs, true, true};
            ser.ExistStream = true;
            asset->Serialize(&ser);

            FileUtil::WriteAllText(assetPhyicsPath, textAssetObject->ToString());

            ResolveDirty(asset);
            asset->SetObjectFlags(asset->GetObjectFlags() | OF_Persistent);
        }

        OnSavedAsset.Invoke(asset);
    }

    void AssetDatabase::SaveAll()
    {
        auto copy = _DirtyObjects;

        for (auto& item : copy)
        {
            Save(item);
        }
    }

    void AssetDatabase::NewAsset(string_view path, Type* assetType)
    {

    }
    static void _WriteAssetToDisk(std::shared_ptr<AssetFileNode> root, string_view path, AssetObject_ref asset)
    {
        const auto newAsset = root->PrepareChildFile(path, ".pmeta");
        newAsset->AssetMeta = mksptr(new AssetMetaData);
        newAsset->AssetMeta->Type = asset->GetType()->GetName();
        newAsset->AssetMeta->Handle = asset.handle;

        ser::JsonSerializerSettings settings;
        settings.IndentSpace = 2;
        const auto json = ser::JsonSerializer::Serialize(newAsset->AssetMeta.get(), settings);
        FileUtil::WriteAllText(newAsset->PhysicsPath, json);
    }

    bool AssetDatabase::CreateAsset(AssetObject_ref asset, string_view path)
    {
        if (ExistAsset(asset))
        {
            return false;
        }
        if (!OnRequestCreateAsset.IsValidReturnInvoke(asset, path))
        {
            return false;
        }

        _WriteAssetToDisk(FileTree, path, asset);
        _AssetRegistry[GetPackageName(path)].AssetPathMapping[asset.handle] = path;

        MarkDirty(asset);
        Save(asset);

        OnCreatedAsset.Invoke(asset);
        return true;
    }

    bool AssetDatabase::DeleteAsset(AssetObject_ref asset)
    {
        if (!ExistAsset(asset))
        {
            return false;
        }
        if (!OnRequestDeleteAsset.IsValidReturnInvoke(asset))
        {
            return false;
        }
        //delete asset
        OnDeletedAsset.Invoke(asset);
        return true;
    }


    void AssetDatabase::MarkDirty(AssetObject_ref asset)
    {
        _DirtyObjects.insert(asset.GetHandle());
    }

    bool AssetDatabase::IsDirty(AssetObject_ref asset)
    {
        return _DirtyObjects.contains(asset.GetHandle());
    }

    Type* AssetFileNode::GetAssetType() const
    {
        if(IsFolder)
        {
            return cltypeof<FolderAsset>();
        }
        if(AssetMeta)
        {
            return AssemblyManager::GlobalFindType(AssetMeta->Type);
        }
        return nullptr;
    }
    void AssetDatabase::Initialize()
    {
        IconPool = std::make_unique<PersistentImagePool>(Application::GetGfxApp());

        FileTree = mksptr(new AssetFileNode);
        FileTree->IsFolder = true;
        FileTree->IsPhysicsFile = false;
        FileTree->AssetName = AssetDatabase::FileTreeRootPath;
        FileTree->AssetPath = "";
        FileTree->IsCollapsed = true;

        Workspace::OnWorkspaceOpened += _OnWorkspaceOpened;
    }

    void AssetDatabase::Refresh()
    {

    }

    void AssetDatabase::Terminate()
    {
        Workspace::OnWorkspaceOpened -= _OnWorkspaceOpened;
        IconPool.reset();
        decltype(_DirtyObjects){}.swap(_DirtyObjects);
        decltype(_AssetRegistry){}.swap(_AssetRegistry);
    }


    static array_list<std::filesystem::path> _PackageSearchPaths;

    void AssetDatabase::AddProgramPackageSearchPath(std::filesystem::path path)
    {
        auto absPath = std::filesystem::absolute(path);
        _PackageSearchPaths.push_back(absPath);
    }

    static array_list<ProgramPackage> _ProgramPackages;

    void AssetDatabase::AddPackage(string_view packageName)
    {
        for (const auto& path : _PackageSearchPaths)
        {
            for (const auto& file : std::filesystem::directory_iterator(path))
            {
                if (file.is_directory() && file.path().filename() == packageName)
                {
                    ProgramPackage package;
                    package.Name = packageName;
                    package.Path = file.path().generic_string();
                    _ProgramPackages.push_back(package);
                    OnAddPackage(&package);
                }
            }
        }
    }

    std::filesystem::path AssetDatabase::GetPackagePath(string_view packageName)
    {
        for (const auto& package : _ProgramPackages)
        {
            if (package.Name == packageName)
            {
                return package.Path;
            }
        }
        return {};
    }
    string AssetDatabase::GetPackageName(string_view assetPath)
    {
        const auto index = assetPath.find('/');
        return string{assetPath.substr(0, index)};
    }

    std::filesystem::path AssetDatabase::GetAbsoluteAssetPath(string_view assetPath)
    {
        const auto index = assetPath.find('/');
        const auto packageName = assetPath.substr(0, index);
        const auto path = assetPath.substr(index + 1);
        return (GetPackagePath(packageName) / path).generic_string();
    }

    array_list<ProgramPackage> AssetDatabase::GetPackageInfos()
    {
        return _ProgramPackages;
    }


    void AssetDatabase::OnAddPackage(ProgramPackage* info)
    {
        std::shared_ptr<AssetFileNode> packageNode = mksptr(new AssetFileNode);
        packageNode->IsFolder = true;
        packageNode->IsPhysicsFile = true;
        packageNode->PhysicsPath = info->Path / "Assets";
        packageNode->AssetName = info->Name;
        packageNode->AssetPath = info->Name;

        _Scan(packageNode, [](std::shared_ptr<AssetFileNode> node) {
            {
                if (node->IsFolder)
                    return;
                string packageName = node->GetPackageName();
                auto it = _AssetRegistry.find(packageName);
                if (it == _AssetRegistry.end())
                {
                    _AssetRegistry.insert({packageName, {}});
                }
                _AssetRegistry[packageName].AssetPathMapping[node->AssetMeta->Handle] = node->AssetPath;
            }
        });

        FileTree->Children.push_back(packageNode);

        // TODO: register meta file
        //auto json = FileUtil::ReadAllText(node->GetPhysicsPath());
        //auto meta = ser::JsonSerializer::Deserialize<AssetMetaData>(json);
    }
}
