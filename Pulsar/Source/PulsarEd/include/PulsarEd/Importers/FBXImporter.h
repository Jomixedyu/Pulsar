#pragma once

#include <PulsarEd/Assembly.h>

namespace pulsared
{
	class FBXImporter
	{
	public:
		static Node_ref Import(string_view path, string& error);
	};
}