#pragma once

#include <ApatiteEd/Assembly.h>

namespace apatiteed
{
	class FBXImporter
	{
	public:
		static Node_sp Import(string_view path);
	};
}