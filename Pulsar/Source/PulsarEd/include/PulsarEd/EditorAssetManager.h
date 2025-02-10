#pragma once
#include <Pulsar/AssetManager.h>
#include <PulsarEd/Assembly.h>
#include <PulsarEd/AssetDatabase.h>


namespace pulsared
{
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
		~EditorAssetManager() override = default;
	};
}