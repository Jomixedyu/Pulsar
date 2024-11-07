#pragma once
#include <Pulsar/AssetManager.h>
#include <PulsarEd/Assembly.h>
#include <PulsarEd/AssetDatabase.h>


#define DEFINE_ASSET_EDITOR(TYPE, ALLOW_DERIVE) static inline struct __asset_editor { __asset_editor() \
{ ::pulsared::EditorAssetManager::Register(cltypeof<TYPE>(), ThisClass::StaticType(), ALLOW_DERIVE); } \
} __asset_editor__;

namespace pulsared
{

	class EditorAssetManager : public pulsar::AssetManager
	{
	public:
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
	public:
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

		virtual RCPtr<AssetObject> LoadAssetAtPath(string_view path) override
		{
			return AssetDatabase::LoadAssetAtPath(path);
		}
		virtual RCPtr<AssetObject> LoadAssetById(ObjectHandle id) override
		{
			return AssetDatabase::LoadAssetById(id);
		}
		~EditorAssetManager() override = default;
	};
}