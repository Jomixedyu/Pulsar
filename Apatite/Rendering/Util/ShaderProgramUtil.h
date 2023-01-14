#pragma once

#include <Apatite/ObjectBase.h>

namespace apatite
{
    class ShaderProgramUtil
    {
    public:

        static std::shared_ptr<ShaderProgramUtil> StaticCreateAndLink(const char* vert, const char* frag);
    };
}