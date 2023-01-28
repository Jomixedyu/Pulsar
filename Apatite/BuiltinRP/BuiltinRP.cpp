#include "BuiltinRP.h"
#include "BuiltinRP.h"
#include <CoreLib/File.h>
#include <Apatite/Application.h>
#include <Apatite/AssetRegistry.h>

namespace apatite::builtinrp
{
    Material_sp BultinRP::GetDefaultMaterial()
    {
        if (this->default_lit_)
        {
            return this->default_lit_;
        }
        auto mat = AssetRegistry::FindAssetAtPath("Engine/Assets/Shaders/BuiltinRP.DefaultLit");
        assert(mat);
        this->default_lit_ = sptr_cast<Material>(mat);
        return this->default_lit_;
    }
    BultinRP::BultinRP()
    {

    }
}