#pragma once

#include <PulsarEd/Assembly.h>
#include <Pulsar/Node.h>

namespace pulsared
{
	struct FBXImporterSettings
	{
		string AssetFolder;
		bool FindMaterial;
		bool ConvertAxisSystem;
	};
	class FBXImporter
	{
	public:
		static Node_ref Import(string_view path, FBXImporterSettings* settings);
	};
}