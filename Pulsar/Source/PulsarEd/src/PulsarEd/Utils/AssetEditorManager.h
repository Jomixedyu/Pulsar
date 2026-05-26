#pragma once
#include <PulsarEd/Assembly.h>
#include <Pulsar/AssetObject.h>
#include <PulsarEd/Editors/EditorRegistry.h>

#define DEFINE_ASSET_EDITOR(TYPE) static inline struct __asset_editor { __asset_editor() \
{ \
    ::pulsared::AssetEditorManager::Register(cltypeof<TYPE>(), ThisClass::StaticType(), false); \
    ::pulsared::EditorRegistry::Register(ThisClass::StaticType()); \
} \
} __asset_editor__;

#define DEFINE_ASSET_EDITOR_DERIVE(TYPE) static inline struct __asset_editor { __asset_editor() \
{ \
    ::pulsared::AssetEditorManager::Register(cltypeof<TYPE>(), ThisClass::StaticType(), true); \
    ::pulsared::EditorRegistry::Register(ThisClass::StaticType()); \
} \
} __asset_editor__;

namespace pulsared
{
    class Editor;

    class AssetEditorManager
    {
    public:
        static void OpenAssetEditor(RCPtr<AssetObject> asset);

        struct Value
        {
            Type* type;
            bool derive;
        };
        static auto& Map()
        {
            static hash_map<Type*, Value> map;
            return map;
        }
        static void Register(Type* k, Type* v, bool allowDerive)
        {
            Value type{};
            type.type = v;
            type.derive = allowDerive;
            Map()[k] = type;
        }
        static void Unregister(Type* k)
        {
            Map().erase(k);
        }
        static Type* GetValue(Type* k)
        {
            auto it = Map().find(k);
            if (it != Map().end())
            {
                return it->second.type;
            }
            else
            {
                for (auto& map : Map())
                {
                    if (map.second.derive && k->IsSubclassOf(map.first))
                    {
                        return map.second.type;
                    }
                }
            }
            return {};
        }


    };
}
