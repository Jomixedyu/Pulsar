#pragma once
#include "MeshDrawPassProcessor.h"
#include "rdg/RDGBuilder.h"

namespace pulsar
{

    class BassPassProcessor : public MeshDrawPassProcessor
    {
        using base = MeshDrawPassProcessor;
    public:
        void Draw(const MeshDrawPassParams& params) override;

        static void BuildBasePass(rdg::RDGBuilder& builder);

    };
} // namespace pulsar