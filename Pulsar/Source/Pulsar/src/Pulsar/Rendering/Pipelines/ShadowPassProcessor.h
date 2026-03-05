#pragma once
#include "MeshDrawPassProcessor.h"

namespace pulsar
{
    class ShadowPassProcessor : public MeshDrawPassProcessor
    {
        using base = MeshDrawPassProcessor;
    public:
        void Draw(const MeshDrawPassParams& params) override;

    };
} // namespace pulsar