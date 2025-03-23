#include <Pulsar/Rendering/SceneRenderer.h>

namespace pulsar::rendering
{

    struct ShaderParameterElementInfo
    {

    };

    struct BasePassParameters : public rdg::ShaderParameters
    {
        rdg::RDGTexture* m_output;

        virtual array_list<ShaderParameterElementInfo> Get() { return {}; }

    };

    void ForwardRenderer::Render(rdg::RDGBuilder& builder)
    {
        rdg::RDGTextureDesc colorDesc{};
        auto colorTex = builder.CreateTexture("Color", colorDesc);

        rdg::RDGTextureDesc depthDesc{};
        auto depthTex = builder.CreateTexture("DepthStencil", depthDesc);

        builder.AddPass("BasePass", rdg::RDGPassFlags::Raster,
            [](rdg::CommandBufferContext& ctx) {

            });

    }
} // namespace pulsar::rendering