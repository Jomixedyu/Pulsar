#pragma once
#include "MeshDrawPassProcessor.h"
#include <rdg/RDGBuilder.h>
#include <rdg/RDGPass.h>

namespace pulsar
{
    struct TriScreenParameters : public rdg::ShaderParameters
    {

    };

    class DepthPassProcessor : public MeshDrawPassProcessor
    {
    };
} // namespace pulsar