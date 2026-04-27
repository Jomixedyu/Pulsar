#pragma once
#include "RenderFeature.h"
#include <Pulsar/Assets/Material.h>
#include <memory>
#include <vector>

namespace pulsar
{
    class CustomPostProcessPass;

    class CustomPostProcessChain : public RenderFeature
    {
    public:
        CustomPostProcessChain();
        ~CustomPostProcessChain();

        void OnSetup(const RenderCaptureContext& ctx) override;
        bool IsEnabled() const override;
        RGTextureHandle AddToGraph(RenderGraph& graph,
                                   RGTextureHandle input,
                                   RGTextureHandle output,
                                   CameraComponent* cam,
                                   PerPassResources* perPass) override;

    private:
        std::vector<std::unique_ptr<CustomPostProcessPass>> m_passPool;
        array_list<RCPtr<Material>> m_materials;
    };
}
