#pragma once
#include <Pulsar/Rendering/Pipelines/Pipeline.h>

namespace pulsar::builtinrp
{
    class BultinRP : public rendering::Pipeline
    {

    public:
        virtual Material_sp GetDefaultMaterial() override;
        BultinRP();

    protected:
        Material_sp default_lit_;
    };
}