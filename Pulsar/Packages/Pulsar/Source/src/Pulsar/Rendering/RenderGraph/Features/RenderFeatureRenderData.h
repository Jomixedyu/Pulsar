#pragma once
#include <memory>

namespace pulsar
{
    class RenderFeatureProxy;

    class RenderFeatureRenderData
    {
    public:
        virtual ~RenderFeatureRenderData() = default;
        virtual std::unique_ptr<RenderFeatureProxy> CreateProxy() const = 0;
    };
}