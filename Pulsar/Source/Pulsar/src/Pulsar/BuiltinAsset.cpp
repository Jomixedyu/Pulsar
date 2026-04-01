#include "BuiltinAsset.h"
#include <unordered_map>
#include "AssetManager.h"

#include "Assets/Shader.h"
#include "Assets/Texture2D.h"


namespace pulsar
{

    static auto& Map()
    {
        static std::unordered_map<const char*, RCPtr<AssetObject>> map;
        return map;
    }

    static RCPtr<AssetObject> GetOrCreate(const char* name)
    {
        auto it = Map().find(name);
        if (it == Map().end())
        {
            auto asset = AssetManager::Get() -> LoadAssetAtPath(name);
            Map().insert({name, asset});
            return asset;
        }
        return it->second;
    }

    RCPtr<Texture2D> BuiltinAsset::GetTextureBlack()
    {
        return cast<Texture2D>(GetOrCreate(Texture_Black));
    }

    void BuiltinAsset::ClearAssets()
    {
        Map().clear();
    }

    const char* BuiltinAsset::ResolveTextureDefault(const string& defaultValue)
    {
        if (defaultValue == "T2D_WHITE")  return Texture_White;
        if (defaultValue == "T2D_BLACK")  return Texture_Black;
        if (defaultValue == "T2D_GRAY")   return Texture_Gray;
        if (defaultValue == "T2D_NORMAL_TS") return Texture_DefaultNormal;
        return nullptr;
    }
}