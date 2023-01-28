#pragma once

#include "ObjectBase.h"
#include "AssetObject.h"

namespace apatite
{


	struct AssetRefBase
	{
	public:

		guid_t guid;
	};

	template<typename T>
	struct AssetRef : public AssetRefBase
	{
	public:
		sptr<T> GetAsset()
		{

		}
	};

	class AssetRefObject : public Object
	{
		CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::AssetRefObject, Object);

	public:

	};
}