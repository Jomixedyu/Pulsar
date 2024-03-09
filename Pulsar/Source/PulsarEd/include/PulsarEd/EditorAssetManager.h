#pragma once
#include <Pulsar/AssetManager.h>
#include <PulsarEd/Assembly.h>
#include <PulsarEd/AssetDatabase.h>


#define DEFINE_ASSET_EDITOR(TYPE) static inline struct __asset_editor { __asset_editor() \
{ ::pulsared::AssetEditorManager::Register(cltypeof<TYPE>(), ThisClass::StaticType()); } \
} __asset_editor__;

namespace pulsared
{
    template<typename T, typename K, typename V>
    class KeyValueRegister
    {
    private:
        static hash_map<K, V>& Map()
        {
            static hash_map<K, V> map;
            return map;
        }
    public:
        static void Register(K k, V v)
        {
            Map()[k] = v;
        }
        static void Unregister(K k)
        {
            Map().erase(k);
        }
        static V GetValue(K k)
        {
            auto it = Map().find(k);
            if (it != Map().end())
            {
                return it->second;
            }
            return {};
        }

    };

    class AssetEditorManager : public KeyValueRegister<AssetEditorManager, Type*, Type*>
    {
    };

	class EditorAssetManager : public pulsar::AssetManager
	{
	public:
		virtual RCPtr<AssetObject> LoadAssetAtPath(string_view path) override
		{
			return AssetDatabase::LoadAssetAtPath(path);
		}
		virtual RCPtr<AssetObject> LoadAssetById(ObjectHandle id) override
		{
			return AssetDatabase::LoadAssetById(id);
		}
		virtual ~EditorAssetManager() override{}
	};
}