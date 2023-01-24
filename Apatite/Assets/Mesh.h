#pragma once
#include <Apatite/AssetObject.h>

namespace apatite
{
    class Mesh : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Mesh, AssetObject);
    public:
        virtual size_t GetVertexCount() = 0;
    };
}