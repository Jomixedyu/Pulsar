#include "PostProcessPass.h"
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Rendering/PerPassResources.h>
#include <Pulsar/Assets/RenderTexture.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>
#include <gfx/GFXDescriptorManager.h>
#include <Pulsar/Application.h>

namespace pulsar
{
    PostProcessPass::PostProcessPass(RCPtr<Material> material)
        : m_material(std::move(material))
    {
    }

    void PostProcessPass::Initialize(PerPassResources* perPass)
    {
        m_perPassSet = perPass->AllocateSet(perPass->GetLayout("PostProcess"));
    }

    void PostProcessPass::Destroy()
    {
        m_perPassSet.reset();
    }

    gfx::GFXDescriptorSetLayout_sp PostProcessPass::GetInputSamplerLayout()
    {
        if (!m_inputSamplerLayout)
        {
            auto* gfxApp = Application::GetGfxApp();
            gfx::GFXDescriptorSetLayoutDesc desc{
                gfx::GFXDescriptorType::CombinedImageSampler,
                gfx::GFXGpuProgramStageFlags::Fragment,
                0, 1
            };
            m_inputSamplerLayout = gfxApp->CreateDescriptorSetLayout(&desc, 1);
        }
        return m_inputSamplerLayout;
    }

    RGTextureHandle PostProcessPass::AddToGraph(RenderGraph& graph,
                                                RGTextureHandle hSrc,
                                                RGTextureHandle hDst,
                                                CameraComponent* cam,
                                                PerPassResources* perPass)
    {
        if (!IsEnabled() || !cam)
            return hSrc;

        PrepareMaterial(cam);
        if (!m_material)
            return hSrc;
        m_material->SubmitParameters();

        auto curSrc = hSrc;
        auto curDst = hDst;

        graph.AddPass(GetPassName())
            .Read(curSrc)
            .Write(curDst, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::DontCare,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
            })
            .WithPerPass(perPass)
            .Prepare([this, perPass](RGPassContext&)
            {
                if (m_material)
                    m_material->PrepareForRendering("PostProcess", "RENDERER_IMAGEPROCESS");
            })
            .Execute([this, curSrc, curDst, cam, perPass]
                     (RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                DrawFullscreen(passCtx, cmdBuffer, curSrc, curDst, cam, perPass);
            });

        return hDst;
    }

    void PostProcessPass::DrawFullscreen(RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer,
                                         RGTextureHandle hSrc, RGTextureHandle hDst,
                                         CameraComponent* cam, PerPassResources* perPass)
    {
        if (!m_material) return;
        auto shader = m_material->GetShader();
        if (!shader || !shader->GetConfig()) return;

        const auto* ppPassBinding = m_material->GetPassBinding("PostProcess", "RENDERER_IMAGEPROCESS")
                                        .m_gpuResourcesInitialized
                                    ? &m_material->GetPassBinding("PostProcess", "RENDERER_IMAGEPROCESS")
                                    : nullptr;
        if (!ppPassBinding) return;

        auto program = ppPassBinding->GetCurrentProgram();
        if (!program) return;

        RenderTexture* dstRT = passCtx.Get(hDst);
        if (!dstRT)
            dstRT = cam->GetRenderTexture().GetPtr();
        if (!dstRT) return;
        auto* dstFBO = dstRT->GetGfxFrameBufferObject().get();
        if (!dstFBO) return;

        // Write per-pass data (Camera + World + SourceTexture) into set 1
        perPass->WriteCameraToSet(m_perPassSet.get());
        perPass->WriteWorldToSet(m_perPassSet.get());

        RenderTexture* srcRT = passCtx.Get(hSrc);
        if (!srcRT)
            srcRT = cam->GetRenderTexture().GetPtr();
        if (srcRT)
        {
            auto srcView = srcRT->GetGfxRenderTarget0();
            if (srcView)
                perPass->WriteTexture(m_perPassSet.get(), 3, srcView.get());
        }
        perPass->Submit(m_perPassSet.get());

        auto* gfxApp = cmdBuffer.GetApplication();
        auto* pipelineMgr = gfxApp->GetGraphicsPipelineManager();
        array_list<gfx::GFXDescriptorSetLayout_sp> descLayouts;
        descLayouts.push_back(ppPassBinding->m_descriptorSetLayout);   // set 0: material
        descLayouts.push_back(m_perPassSet->GetDescriptorSetLayout()); // set 1: per-pass (Camera/World/Source)

        auto& gpuPrograms = program->GetGpuPrograms();

        gfx::GFXGraphicsPipelineStateParams psoParams{};
        psoParams.DepthTestEnable  = false;
        psoParams.DepthWriteEnable = false;
        psoParams.CullMode         = gfx::GFXCullMode::None;

        auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(
            gpuPrograms, psoParams, descLayouts,
            dstFBO->GetRenderTargetDesc(), {});
        if (!gfxPipeline) return;

        cmdBuffer.CmdSetViewport(0, 0, (float)dstFBO->GetWidth(), (float)dstFBO->GetHeight());
        cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());

        array_list<gfx::GFXDescriptorSet*> descSets;
        descSets.push_back(ppPassBinding->m_descriptorSet.get()); // set 0
        descSets.push_back(m_perPassSet.get());                    // set 1
        cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get());

        cmdBuffer.CmdDraw(3);
    }
}
