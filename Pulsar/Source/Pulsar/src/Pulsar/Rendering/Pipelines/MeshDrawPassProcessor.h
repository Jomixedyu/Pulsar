#pragma once
#include "PassProcessor.h"

namespace pulsar
{
    struct MeshDrawPassParams
    {

    };
    class MeshDrawPassProcessor : public PassProcessor
    {
    public:
        virtual void Draw(const MeshDrawPassParams& params) {}
    };
} // namespace pulsar