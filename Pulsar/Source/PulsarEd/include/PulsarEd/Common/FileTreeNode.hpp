#pragma once
#include <CoreLib/File.h>
#include <filesystem>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <CoreLib/UString.h>

namespace pulsared
{
    template <typename T>
    struct FileTreeNode : public std::enable_shared_from_this<T>
    {
        using string = std::string;
        using string_view = std::string_view;
        using sharedthis = std::enable_shared_from_this<T>;
    public:
        std::filesystem::path PhysicsPath;
        string AssetName;
        string AssetPath;
        bool IsFolder;
        bool IsPhysicsFile;
        bool IsCreated;

        bool Valid;
        std::weak_ptr<T> Parent;
    protected:
        std::vector<std::shared_ptr<T>> Children;
    public:

        const std::vector<std::shared_ptr<T>>& GetChildren() const
        {
            return Children;
        }
        string GetPhysicsPath() const
        {
            return jxcorlib::StringUtil::StringCast(absolute(PhysicsPath).generic_u8string());
        }
        string GetPhysicsName() const
        {
            return jxcorlib::StringUtil::StringCast(PhysicsPath.filename().generic_u8string());
        }
        string GetPhysicsNameWithoutExt() const
        {
            return  jxcorlib::StringUtil::StringCast(PhysicsPath.filename().replace_extension().generic_u8string());
        }
        string GetPhysicsNameExt() const
        {
            return IsFolder ? string{} :  jxcorlib::StringUtil::StringCast(PhysicsPath.extension().generic_u8string());
        }
        string GetRootName() const
        {
            return AssetPath.substr(0, AssetPath.find('/'));
        }

        std::shared_ptr<T> GetChild(string_view name)
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

        std::shared_ptr<T> Find(string_view path)
        {
            if (path == "")
            {
                return sharedthis::shared_from_this();
            }
            std::shared_ptr<T> child = sharedthis::shared_from_this();
            for(auto item : path | std::views::split('/'))
            {
                if (!((child = child->GetChild(string_view{item}))))
                {
                    return nullptr;
                }
            }
            return child;
        }
        void AddChild(const std::shared_ptr<T>& child)
        {
            child->Parent = sharedthis::shared_from_this();
            Children.push_back(child);
        }
        std::shared_ptr<T> NewChild()
        {
            auto newChild = std::make_shared<T>();
            newChild->Parent = sharedthis::shared_from_this();
            Children.push_back(newChild);
            return newChild;
        }
        void RemoveChild(const std::shared_ptr<T>& node)
        {
            auto it = std::ranges::find(Children, node);
            if (it != Children.end())
            {
                Children.erase(it);
            }
        }
        std::shared_ptr<T> CreateChildFolder(string_view name)
        {
            auto newChild = std::make_shared<T>();
            newChild->IsFolder = true;
            newChild->AssetName = name;
            newChild->AssetPath = AssetPath + "/" + string{name};
            newChild->Parent = sharedthis::shared_from_this();
            Children.push_back(newChild);
            Sort();
            return newChild;
        }
        std::shared_ptr<T> CreateChildFile(string_view name, string_view ext)
        {
            auto newChild = std::make_shared<T>();
            newChild->IsFolder = false;
            newChild->Parent = sharedthis::shared_from_this();
            newChild->AssetName = name;
            newChild->AssetPath = AssetPath + "/" + string{name};
            newChild->PhysicsPath = PhysicsPath / jxcorlib::u8path(string{name} + string{ext});
            Children.push_back(newChild);
            Sort();
            return newChild;
        }

        std::shared_ptr<T> PrepareChildFile(string_view path, string_view ext)
        {
            if (path == "")
            {
                return sharedthis::shared_from_this();
            }
            auto child = sharedthis::shared_from_this();

            auto splits = path
                | std::views::split('/')
                | std::ranges::to<std::vector<string>>();

            for(size_t i = 0; auto item : splits)
            {
                auto curChild = child->GetChild(item);
                if (!curChild)
                {
                    if (i != splits.size() - 1)
                    {
                        curChild = child->CreateChildFolder(item);
                    }
                    else
                    {
                        curChild = child->CreateChildFile(item, ext);
                    }
                }
                child = curChild;
                ++i;
            }
            return child;
        }
        void Sort()
        {
            using element = const decltype(Children)::value_type&;
            auto func = [](element a, element b)
            {
                if((a->IsFolder && b->IsFolder) || (!a->IsFolder && !b->IsFolder))
                {
                    return a->AssetName < b->AssetName;
                }
                else if(a->IsFolder && !b->IsFolder)
                {
                    return true;
                }
                else if(!a->IsFolder && b->IsFolder)
                {
                    return false;
                }
                return false;
            };

            std::ranges::sort(Children, func);
        }
    };
}
