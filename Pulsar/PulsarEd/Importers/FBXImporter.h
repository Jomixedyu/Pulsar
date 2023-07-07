#pragma once

#include <PulsarEd/Assembly.h>

namespace pulsared
{
	class FBXImporter
	{
	public:
		static Node_sp Import(string_view path);
	};
}