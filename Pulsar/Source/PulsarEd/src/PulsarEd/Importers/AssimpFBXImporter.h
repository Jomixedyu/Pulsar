#pragma once

#include <PulsarEd/Assembly.h>
#include <Pulsar/Node.h>

namespace pulsared
{
	class AssimpFBXImporter
	{
	public:
		static ObjectPtr<Node> Import(string_view path, string& error);
	};
}