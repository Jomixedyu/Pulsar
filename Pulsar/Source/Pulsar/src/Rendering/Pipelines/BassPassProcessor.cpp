#include <Pulsar/Rendering/Pipelines/BassPassProcessor.h>

namespace pulsar
{

    void BassPassProcessor::Draw(const MeshDrawPassParams& params)
    {
        base::Draw(params);
    }


    struct BasePassParams : public rdg::ShaderParameters
    {
        float f;
    };

    struct RDGRenderPassDesc
    {

    };

    void BassPassProcessor::BuildBasePass(rdg::RDGBuilder& builder)
    {
        BasePassParams params;
        params.f = 0.5f;

        rdg::RDGTextureDesc texDesc{};
        texDesc.Width = 1024;
        texDesc.Height = 1024;
        texDesc.DataType = gfx::GFXTextureDataType::Texture2D;
        texDesc.Format = gfx::GFXTextureFormat::R8G8B8A8_UNorm;

        auto output = builder.CreateTexture("BasePassOutput", texDesc);

        builder.AddPass(
            "BasePass",
            rdg::RDGPassFlags::Raster,
            [output](rdg::CommandBufferContext& ctx) {
                ctx.Cmd.CmdBeginRenderPass("BasePass");

                // bind all
                // draw
                ctx.Cmd.CmdEndRenderPass();
            });

    }


} // namespace pulsar