#pragma once
#include <Apatite/Rendering/Pipelines/Pipeline.h>

namespace apatite::builtinrp
{
    class BultinRP : public rendering::Pipeline
    {

    public:
        virtual Material_sp GetDefaultMaterial() override { return default_lit_; }
        BultinRP();

    protected:
        Material_sp default_lit_;
    };
}