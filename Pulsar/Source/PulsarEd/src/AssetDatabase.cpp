#include "AssetDatabase.h"
#include "Workspace.h"
#include "AssetProviders/AssetProvider.h"
#include <filesystem>
#include <unordered_set>
#include <PulsarEd/AssetProviders/AssetProvider.h>
#include <CoreLib/File.h>
#include <CoreLib.Serialization/JsonSerializer.h>
#include <gfx/GFXImage.h>
#include <fstream>

namespace pulsared
{
    string AssetFileNode::GetPhysicsPath() const
    {
        return StringUtil::StringCast(std::filesystem::absolute(this->PhysicsPath).generic_u8string());
    }

    string AssetFileNode::GetPhysicsName() const
    {
        return StringUtil::StringCast(this->PhysicsPath.filename().generic_u8string());
    }

    string AssetFileNode::GetPhysicsNameWithoutExt() const
    {
        return StringUtil::StringCast(this->PhysicsPath.filename().replace_extension().generic_u8string());
    }

    string AssetFileNode::GetPhysicsNameExt() const
    {
        if (this->IsFolder)
        {
            return {};
        }
        return this->PhysicsPath.extension().string();
    }

    string AssetFileNode::GetPackageName() const
    {
        return AssetPath.substr(0, AssetPath.find('/'));
    }

    Type* AssetFileNode::GetAssetType() const
    {
        if (IsFolder)
        {
            return cltypeof<FolderAsset>();
        }
        return AssemblyManager::GlobalFindType(AssetMeta->Type);
    }

    std::shared_ptr<AssetFileNode> AssetFileNode::GetChild(string_view name)
    {
        for (auto& child : Children)
        {
            if (child->AssetName == name)
            {
                return child;
            }
        }
        return nullptr;
    }

    std::shared_ptr<AssetFileNode> AssetFileNode::Find(string_view path)
    {
        if (path == "")
        {
            return shared_from_this();
        }
        auto child = shared_from_this();
        for (auto item : StringUtil::Split(path, '/'))
        {
            if (!(child = child->GetChild(item)))
            {
                return nullptr;
            }
        }
        return child;
    }


    void AssetIconPool::Register(const index_string& id, const uint8_t* iconBuf, size_t length)
    {
        gfx::GFXSamplerConfig config;

        int32_t width, height, channel;
        auto iconData = gfx::LoadImageFromMemory(iconBuf, length, &width, &height, &channel, 4, true);

        auto tex2d = Application::GetGfxApp()->CreateFromImageData(iconData.data(), width, height, channel, false, gfx::GFXTextureFormat::R8G8B8A8, config);
        m_textures.emplace(id, tex2d);
    }

    gfx::GFXDescriptorSet_wp AssetIconPool::GetDescriptorSet(const index_string& id)
    {
        //read cache
        auto it = m_cacheDescSets.find(id);
        if (it != m_cacheDescSets.end())
        {
            return it->second;
        }

        //generate cache
        auto texIt = m_textures.find(id);
        if (texIt == m_textures.end())
        {
            return {};
        }
        auto tex = texIt->second;
        auto descSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descriptorLayout.get());

        auto desc = descSet->FindByBinding(0);
        if (!desc)
            desc = descSet->AddDescriptor("p", 0);

        desc->SetTextureSampler2D(tex.get());

        descSet->Submit();

        m_cacheDescSets.emplace(id, descSet);

        return descSet;
    }

    gfx::GFXDescriptorSet_wp AssetIconPool::GetDescriptorSet(Type* id)
    {
        return GetDescriptorSet(index_string{id->GetName()});
    }

    void AssetIconPool::ClearCache()
    {
        m_cacheDescSets.clear();
    }

    AssetIconPool::AssetIconPool()
    {
        m_descriptorLayout = Application::GetGfxApp()->CreateDescriptorSetLayout({
            gfx::GFXDescriptorSetLayoutInfo(0, gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXShaderStageFlags::Fragment)
        });
    }

    AssetIconPool::~AssetIconPool()
    {

    }


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

        auto json = FileUtil::ReadAllText(node->GetPhysicsPath());
        auto meta = ser::JsonSerializer::Deserialize<AssetMetaData>(json);

        if (auto existObj = RuntimeObjectWrapper::GetObject(meta->Handle))
        {
            return (AssetObject*)existObj;
        }

        auto type = AssemblyManager::GlobalFindType(meta->Type);
        auto obj = sptr_cast<AssetObject>(type->CreateSharedInstance({}));
        assert(obj);

        obj->SetName(node->AssetName);
        obj->Construct(meta->Handle);

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
            obj->Serialize(&serializer);
        }

        return obj;
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
        // add new asset package
        // save & serialize

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

    void AssetDatabase::Initialize()
    {
        IconPool = std::make_unique<AssetIconPool>();

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

    std::filesystem::path AssetDatabase::GetAbsoluteAssetPath(string_view assetPath)
    {
        auto index = assetPath.find('/');
        auto packageName = assetPath.substr(0, index);
        auto path = assetPath.substr(index + 1);
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
