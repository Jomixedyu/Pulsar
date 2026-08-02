#include "PostProcessPass.h"
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Rendering/RenderScene.h>
#include <Pulsar/Rendering/RenderResourceRegistry.h>
#include <Pulsar/Rendering/DescriptorSetCache.h>
#include <Pulsar/Rendering/DescriptorSetAssembler.h>
#include <Pulsar/Assets/RenderTexture.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>
#include <Pulsar/Application.h>

namespace pulsar
{
    PostProcessPass::PostProcessPass(RCPtr<Material> material)
        : m_material(std::move(material))
    {
    }

    void PostProcessPass::Initialize()
    {
    }

    void PostProcessPass::Destroy()
    {
    }

    gfx::GFXDescriptorSetLayout_sp PostProcessPass::GetInputSamplerLayout()
    {
        if (!m_inputSamplerLayout)
        {
            auto* gfxApp = Application::GetGfxApp();
            gfx::GFXDescriptorLayoutDesc desc{
                gfx::GFXDescriptorType::CombinedImageSampler,
                gfx::GFXGpuProgramStageFlags::Fragment,
                0, 1
            };
            m_inputSamplerLayout = gfxApp->GetOrCreateDescriptorSetLayout(&desc, 1);
        }
        return m_inputSamplerLayout;
    }

    RGTextureHandle PostProcessPass::AddToGraph(RenderGraph& graph,
                                                RGTextureHandle hSrc,
                                                RGTextureHandle hDst,
                                                const RenderCaptureContext& ctx)
    {
        if (!IsEnabled() || !ctx.view)
            return hSrc;

        PrepareMaterial(ctx);
        if (!m_material)
            return hSrc;
        m_material->SubmitParameters();
        m_proxy = m_material->GetRenderProxy();
        if (!m_proxy)
            return hSrc;

        auto curSrc = hSrc;
        auto curDst = hDst;

        gfx::GFXFrameBufferObject_sp fallbackFBO  = ctx.view->RenderTarget.Framebuffer;
        gfx::GFXTexture2DView_sp     fallbackView = ctx.view->RenderTarget.GetRenderTarget0();

        gfx::GFXBuffer* cameraBuffer = ctx.viewProxy ? ctx.viewProxy->GetCameraBuffer() : nullptr;
        gfx::GFXBuffer* worldBuffer  = ctx.scene ? ctx.scene->GetWorldBuffer() : nullptr;

        graph.AddPass(GetPassName())
            .Read(curSrc)
            .Write(curDst, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::DontCare,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
            })
            .Prepare([this](RGPassContext&)
            {
                if (m_proxy)
                    m_proxy->ResolveRenderVariant("PostProcess", "RENDERER_IMAGEPROCESS");
            })
            .Execute([this, curSrc, curDst, fallbackFBO, fallbackView, cameraBuffer, worldBuffer]
                     (RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                DrawFullscreen(passCtx, cmdBuffer, curSrc, curDst, fallbackFBO, fallbackView, cameraBuffer, worldBuffer);
            });

        return hDst;
    }

    void PostProcessPass::DrawFullscreen(RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer,
                                         RGTextureHandle hSrc, RGTextureHandle hDst,
                                         const gfx::GFXFrameBufferObject_sp& fallbackFBO,
                                         const gfx::GFXTexture2DView_sp& fallbackView,
                                         gfx::GFXBuffer* cameraBuffer,
                                         gfx::GFXBuffer* worldBuffer)
    {
        if (!m_proxy) return;
        if (!m_proxy->GetShaderConfig()) return;

        auto resolved = m_proxy->ResolveRenderVariant("PostProcess", "RENDERER_IMAGEPROCESS");
        if (!resolved) return;
        auto program = resolved.m_program;
        if (program->GetGpuPrograms().empty()) return;

        const auto* dstRT = passCtx.Get(hDst);
        gfx::GFXFrameBufferObject* dstFBO = nullptr;
        if (dstRT)
        {
            dstFBO = dstRT->GetFrameBufferObject().get();
        }
        else if (fallbackFBO)
        {
            dstFBO = fallbackFBO.get();
        }
        if (!dstFBO) return;

        const auto* srcRT = passCtx.Get(hSrc);
        gfx::GFXTexture2DView* srcView = nullptr;
        if (srcRT)
        {
            srcView = srcRT->GetRenderTarget0().get();
        }
        else if (fallbackView)
        {
            srcView = fallbackView.get();
        }

        // Build the per-pass registry and resolve set 1 via the content-addressed cache.
        RenderResourceRegistry reg;
        reg.Set("CameraBuffer", cameraBuffer);
        reg.Set("WorldBuffer", worldBuffer);
        reg.Set("PP_InColor", srcView);

        const ShaderPropertySetLayout* set1 = program->m_layout.FindSet(1);
        auto set1Layout = DescriptorSetAssembler::BuildLayout(set1);
        gfx::GFXDescriptorSet* perPassSet = DescriptorSetCache::Instance().Get(set1Layout, set1, reg);
        if (!perPassSet) return;

        auto* gfxApp = cmdBuffer.GetApplication();
        auto* pipelineMgr = gfxApp->GetGraphicsPipelineManager();
        array_list<gfx::GFXDescriptorSetLayout_sp> descLayouts;
        descLayouts.push_back(resolved.m_set0Layout); // set 0: material
        descLayouts.push_back(set1Layout);            // set 1: per-pass (Camera/World/Source)

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
        descSets.push_back(resolved.m_set0); // set 0
        descSets.push_back(perPassSet);      // set 1
        cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get());

        cmdBuffer.CmdDraw(3);
    }
}
