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
#include <utility>

namespace pulsared
{

    static std::unordered_set<RCPtr<AssetObject>> _DirtyObjects;

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
            newNode->AssetName = StringUtil::StringCast(i.path().stem().u8string());
            newNode->AssetPath = node->AssetPath + '/' + newNode->AssetName;
            newNode->IsCreated = true;

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
            node->AddChild(newNode);
        }
        node->Sort();
    }

    static void _OnWorkspaceOpened()
    {

    }

    static std::array<std::filesystem::path, 3> _GetClusterPhysicPath(const std::filesystem::path path)
    {
        auto physic = path;
        physic.replace_extension();

        auto physicMeta = physic;
        physicMeta.replace_extension(".pmeta");

        auto physicAsset = physic;
        physicAsset.replace_extension(".pa");

        auto physicBinary = physic;
        physicBinary.replace_extension(".pba");

        return { physicMeta, physicAsset, physicBinary };
    }

    RCPtr<AssetObject> AssetDatabase::LoadAssetAtPath(string_view path)
    {
        // string pathStr{path};

        if(auto id = GetIdByPath(path))
        {
            if(auto obj = RuntimeObjectManager::GetObject(id))
            {
                return obj;
            }
        }

        auto node = FileTree->Find(path).get();
        if (!node)
        {
            return nullptr;
        }
        if (!node->IsCreated)
        {
            return GetIdByPath(path);
        }

        SPtr<AssetObject> assetObj;

        auto json = FileUtil::ReadAllText(node->GetPhysicsPath());
        auto meta = ser::JsonSerializer::Deserialize<AssetMetaData>(json);

        if (auto existObj = RuntimeObjectManager::GetObject(meta->Handle))
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

    RCPtr<AssetObject> AssetDatabase::LoadAssetById(ObjectHandle id)
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
    bool AssetDatabase::ExistsAssetPath(string_view path)
    {
        string pathStr{path};

        auto node = FileTree->Find(path).get();
        return node != nullptr;
    }

    string AssetDatabase::GetPathById(ObjectHandle id)
    {
        for (auto& [package, registry] : _AssetRegistry)
        {
            auto it = registry.AssetPathMapping.find(id);
            if (it != registry.AssetPathMapping.end())
            {
                return it->second;
            }
        }

        return {};
    }
    string AssetDatabase::GetPathByAsset(const RCPtr<AssetObject>& asset)
    {
        return GetPathById(asset.GetHandle());
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
    string AssetDatabase::GetUniquePath(string_view path)
    {
        string p {path};
        while (ExistsAssetPath(p))
        {
            p += "1";
        }
        return p;
    }


    void AssetDatabase::ResolveDirty(const RCPtr<AssetObject>& asset) noexcept
    {
        auto it = _DirtyObjects.find(asset.GetHandle());
        if (it != _DirtyObjects.end())
        {
            _DirtyObjects.erase(it);
        }
    }

    bool AssetDatabase::ExistsAsset(const RCPtr<AssetObject>& asset)
    {
        return !GetPathByAsset(asset).empty();
    }

    void AssetDatabase::ReloadAsset(ObjectHandle id)
    {
        RuntimeObjectManager::DestroyObject(id, true);
        LoadAssetById(id);
        ResolveDirty(id);
    }

    void AssetDatabase::Save(const RCPtr<AssetObject>& asset)
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

            ser::JsonSerializerSettings metaJsonSettings{};
            metaJsonSettings.IndentSpace = 4;
            auto metaJson = ser::JsonSerializer::Serialize(node->AssetMeta.get(), metaJsonSettings);
            FileUtil::WriteAllText(node->PhysicsPath, metaJson);

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
            node->IsPhysicsFile = true;

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
            Save(item.GetPtr());
        }
    }

    void AssetDatabase::NewAsset(string_view folderPath, string_view assetName, Type* assetType)
    {
        Logger::Log("new asset : " + string{folderPath} + " ; " + string{assetName});

        auto attr = assetType->GetAttribute<CreateAssetAttribute>(false);

        RCPtr<AssetObject> asset;
        if (attr && attr->GetInstantiatePath())
        {
            auto _asset = LoadAssetAtPath(attr->GetInstantiatePath());
            asset = _asset->InstantiateAsset();
        }
        else
        {
            asset = sptr_cast<AssetObject>(assetType->CreateSharedInstance({}));
            asset->Construct();
        }

        CreateAsset(asset, GetUniquePath(string{folderPath} + "/" + string{assetName}));
    }

    static void _WriteAssetToDisk(
        const std::shared_ptr<AssetFileNode>& root,
        string_view path,
        RCPtr<AssetObject> asset)
    {
        const auto newAsset = root->PrepareChildFile(path, ".pmeta");
        newAsset->AssetMeta = mksptr(new AssetMetaData);
        newAsset->AssetMeta->Type = asset->GetType()->GetName();
        newAsset->AssetMeta->Handle = asset.GetHandle();
        newAsset->IsPhysicsFile = false;
        //newAsset->PhysicsPath = phypath;
        // ser::JsonSerializerSettings settings;
        // settings.IndentSpace = 2;
        // const auto json = ser::JsonSerializer::Serialize(newAsset->AssetMeta.get(), settings);
        // FileUtil::WriteAllText(newAsset->PhysicsPath, json);
    }

    bool AssetDatabase::CreateAsset(const RCPtr<AssetObject>& asset, string_view path)
    {
        if (ExistsAsset(asset))
        {
            return false;
        }
        if (!OnRequestCreateAsset.IsValidReturnInvoke(asset, path))
        {
            return false;
        }

        _WriteAssetToDisk(FileTree, path, asset);
        _AssetRegistry[GetPackageName(path)].AssetPathMapping[asset->GetObjectHandle()] = path;

        MarkDirty(asset);

        OnCreatedAsset.Invoke(asset);
        return true;
    }

    bool AssetDatabase::DeleteAssets(const array_list<string>& assetPaths, array_list<string>* errinfo)
    {
        namespace fs = std::filesystem;

        array_list<AssetFileNodePtr> nodes;
        nodes.reserve(assetPaths.size());

        // check
        for (auto path : assetPaths)
        {
            auto node = FileTree->Find(path);
            if (node == nullptr)
            {
                nulable$(errinfo)->push_back("not found:" + string{path});
                return false;
            }
            if (node->IsFolder)
            {
                nulable$(errinfo)->push_back("existing folder:" + string{path});
                return false;
            }
            if (!OnRequestDeleteAsset.IsValidReturnInvoke(path, errinfo))
            {
                return false;
            }
            nodes.push_back(node);
        }

        for (const auto& node : nodes)
        {
            string packageName = node->GetPackageName();
            string assetPath = node->AssetPath;

            OnDeletedAsset.Invoke(assetPath);
            auto physicPath = AssetPathToPhysicsPath(assetPath);

            // remove virtual file tree
            auto parentNode = node->Parent.lock();
            parentNode->RemoveChild(node);

            // destroy memory object
            auto handle = GetIdByPath(assetPath);
            if (RuntimeObjectManager::IsValid(handle))
            {
                ObjectPtr<AssetObject> asset = RuntimeObjectManager::GetObject(handle)->GetObjectHandle();
                DestroyObject(asset, true);
            }

            // unregister registry
            auto assetId = GetIdByPath(assetPath);
            _AssetRegistry.at(packageName).AssetPathMapping.erase(assetId);

            // delete physic file
            for (auto& removePath : _GetClusterPhysicPath(physicPath))
            {
                if (std::filesystem::exists(removePath))
                {
                    std::filesystem::remove(removePath);
                }
            }

        }


        return true;
    }

    void AssetDatabase::DeleteEmptyFolder(string_view path)
    {
        auto node = FileTree->Find(path);
        if (node)
        {
            node->Parent.lock()->RemoveChild(node);
            if (std::filesystem::exists(node->PhysicsPath))
            {
                std::filesystem::remove(node->PhysicsPath);
            }
        }
    }

    void AssetDatabase::DeleteFolder(string_view path, array_list<string>* errinfo)
    {
        AssetFilter filter;
        filter.BlackList.push_back(cltypeof<FolderAsset>());
        filter.FolderPath = path;

        if (DeleteAssets(FindAssets(filter), errinfo))
        {
            DeleteEmptyFolder(path);
        }
    }

    static void _FindAssets(array_list<string>& paths, AssetFileNode* node, const AssetFilter& filter)
    {
        if (filter.FolderPath.empty() || node->Parent.lock()->AssetPath == filter.FolderPath)
        {
            if (!filter.WhiteList.empty())
            {
                if (std::ranges::contains(filter.WhiteList, node->GetAssetType()))
                {
                    paths.push_back(node->AssetPath);
                }
            }
            else if(!filter.BlackList.empty())
            {
                if (!std::ranges::contains(filter.BlackList, node->GetAssetType()))
                {
                    paths.push_back(node->AssetPath);
                }
            }
            else
            {
                paths.push_back(node->AssetPath);
            }
        }

        for (auto child : node->GetChildren())
        {
            _FindAssets(paths, child.get(), filter);
        }
    }
    array_list<string> AssetDatabase::FindAssets(const AssetFilter& filter)
    {
        array_list<string> ret;
        for (const auto& package : FileTree->GetChildren())
        {
            _FindAssets(ret, package.get(), filter);
        }
        return ret;
    }
    array_list<string> AssetDatabase::FindAssets(Type* type, string_view folderPath)
    {
        AssetFilter filter;
        filter.WhiteList.push_back(type);
        filter.WhiteList.push_back(type);
        filter.FolderPath = folderPath;
        return FindAssets(filter);
    }
    static bool _IsEmptyFolder(const AssetFileNodePtr& folderNode)
    {
        if (folderNode == nullptr)
        {
            return false;
        }

        for (const auto& child : folderNode->GetChildren())
        {
            if (!child->IsFolder || !_IsEmptyFolder(child))
            {
                return false;
            }
        }

        return true;
    }

    bool AssetDatabase::IsEmptyFolder(string_view path) noexcept
    {
        if (auto node = FileTree->Find(path))
        {
            return _IsEmptyFolder(node);
        }
        return false;
    }

    static bool _RenameAsset(string_view srcAsset, string_view dstAsset)
    {
        auto destNode = AssetDatabase::FileTree->Find(dstAsset);
        auto node = AssetDatabase::FileTree->Find(srcAsset);

        if (destNode || !node)
        {
            return false;
        }


        auto srcFiles = _GetClusterPhysicPath(node->PhysicsPath);

        auto dstFile = AssetDatabase::AssetPathToPhysicsPath(dstAsset);

        if (node->IsPhysicsFile)
        {
            for (int i = 0; i < srcFiles.size(); ++i)
            {
                auto target = dstFile;
                target.replace_extension(srcFiles[i].extension());
                std::filesystem::rename(srcFiles[i], target);
            }
        }

        auto dstFolder = AssetDatabase::AssetPathToParentPath(dstAsset);
        auto dstFolderNode = AssetDatabase::FileTree->Find(dstFolder);

        node->Parent.lock()->RemoveChild(node);
        node->PhysicsPath = AssetDatabase::AssetPathToPhysicsPath(dstAsset).replace_extension(".pmeta");
        node-> SetAssetPath(dstAsset);
        dstFolderNode->AddChild(node);

        auto packageName = AssetDatabase::GetPackageName(srcAsset);
        _AssetRegistry.at(packageName).AssetPathMapping.at(node->AssetMeta->Handle) = dstAsset;


        return true;
    }

    bool AssetDatabase::Rename(string_view srcAsset, string_view dstAsset)
    {
        auto destNode = FileTree->Find(dstAsset);
        auto node = FileTree->Find(srcAsset);

        if (destNode || !node)
        {
            return false;
        }

        // todo : if srcAsset is folder
        if (IsFolderPath(srcAsset))
        {

            return false;
        }

        return _RenameAsset(srcAsset, dstAsset);
    }

    void AssetDatabase::MarkDirty(const RCPtr<AssetObject>& asset) noexcept
    {
        _DirtyObjects.insert(asset);
    }

    bool AssetDatabase::IsDirty(const RCPtr<AssetObject>& asset) noexcept
    {
        return _DirtyObjects.contains(asset);
    }
    bool AssetDatabase::IsDirtyHandle(const ObjectHandle& asset) noexcept
    {
        return std::ranges::any_of(_DirtyObjects, [asset](auto& obj) { return obj.Handle == asset; });
    }

    Type* AssetFileNode::GetAssetType() const
    {
        if (IsFolder)
        {
            return cltypeof<FolderAsset>();
        }
        if (AssetMeta)
        {
            return AssemblyManager::GlobalFindType(AssetMeta->Type);
        }
        return nullptr;
    }
    static void _OnPostEditChanged(ObjectBase* object)
    {
        if(cltypeof<AssetObject>()->IsInstanceOfType(object))
        {
            AssetDatabase::MarkDirty(object->GetObjectHandle());
        }
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
        RuntimeObjectManager::OnPostEditChanged += _OnPostEditChanged;
    }

    void AssetDatabase::Terminate()
    {
        Workspace::OnWorkspaceOpened -= _OnWorkspaceOpened;
        RuntimeObjectManager::OnPostEditChanged -= _OnPostEditChanged;
        IconPool.reset();
        decltype(_DirtyObjects){}.swap(_DirtyObjects);
        decltype(_AssetRegistry){}.swap(_AssetRegistry);
    }

    void AssetDatabase::Refresh()
    {

    }

    static array_list<std::filesystem::path> _PackageSearchPaths;

    void AssetDatabase::AddProgramPackageSearchPath(const std::filesystem::path& path)
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

    std::filesystem::path AssetDatabase::GetPackagePhysicsPath(string_view packageName)
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

    std::filesystem::path AssetDatabase::PackagePathToPhysicsPath(string_view packagePath)
    {
        const auto index = packagePath.find('/');
        if (index == string_view::npos) // root
        {
            return GetPackagePhysicsPath(packagePath);
        }
        const auto packageName = packagePath.substr(0, index);
        const auto path = packagePath.substr(index + 1);
        return (GetPackagePhysicsPath(packageName) / path).generic_string();
    }
    string AssetDatabase::AssetPathToPackagePath(string_view assetPath)
    {
        const auto index = assetPath.find('/');
        auto packageName = assetPath.substr(0, index);
        string path;
        if(index != string_view::npos)
        {
            path += string{"/"};
            path += assetPath.substr(index + 1);
        }
        return string{packageName} + "/Assets" + path;
    }
    std::filesystem::path AssetDatabase::AssetPathToPhysicsPath(string_view assetPath)
    {
        return PackagePathToPhysicsPath(AssetPathToPackagePath(assetPath));
    }
    string AssetDatabase::AssetPathToAssetName(string_view assetPath)
    {
        auto index = assetPath.rfind('/');
        if (index != string_view::npos)
        {
            return string{assetPath.substr(index + 1)};
        }
        return string{assetPath};
    }
    string AssetDatabase::AssetPathToParentPath(string_view path)
    {
        auto index = path.rfind('/');
        if (index != string_view::npos)
        {
            return string{path.substr(0, index)};
        }
        return string{path};
    }
    bool AssetDatabase::IsFolderPath(string_view assetPath)
    {
        auto node = FileTree->Find(assetPath);
        if (node && node->IsFolder)
        {
            return true;
        }
        return false;
    }

    array_list<ProgramPackage> AssetDatabase::GetPackageInfos()
    {
        return _ProgramPackages;
    }


    void AssetDatabase::OnAddPackage(ProgramPackage* info)
    {
        std::shared_ptr<AssetFileNode> packageNode = FileTree->NewChild();
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

        // TODO: register meta file
        //auto json = FileUtil::ReadAllText(node->GetPhysicsPath());
        //auto meta = ser::JsonSerializer::Deserialize<AssetMetaData>(json);
    }
}
