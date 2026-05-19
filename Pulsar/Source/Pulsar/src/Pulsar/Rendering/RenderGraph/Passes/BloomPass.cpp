#include "BloomPass.h"
#include <Pulsar/Assets/BloomSettings.h>
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Assets/RenderTexture.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/AssetManager.h>
#include <Pulsar/Application.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>
#include <gfx/GFXDescriptorManager.h>
#include <gfx/TextureClasses.h>

namespace pulsar
{
    BloomPass::BloomPass()
    {
        auto shader = AssetManager::Get()->LoadAsset<Shader>("Project/Shaders/NapBloom");
        if (!shader)
            shader = AssetManager::Get()->LoadAsset<Shader>("Shaders/NapBloom");
        if (!shader)
            return;

        m_material = Material::StaticCreate(shader);
        m_material->CreateGPUResource();
    }

    BloomPass::~BloomPass()
    {
        Destroy();
    }

    void BloomPass::Initialize()
    {
        auto gfxApp = Application::GetGfxApp();
        auto* descMgr = gfxApp->GetDescriptorManager();

        // Layout for Downsample / GaussFilter: binding 3 (texture) + binding 8 (cbuffer)
        {
            std::vector<gfx::GFXDescriptorSetLayoutDesc> descs = {
                {gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXGpuProgramStageFlags::Fragment, 3, 1},
                {gfx::GFXDescriptorType::ConstantBuffer,       gfx::GFXGpuProgramStageFlags::Fragment, 8, 1},
            };
            m_bloomLayout = gfxApp->CreateDescriptorSetLayout(descs.data(), static_cast<uint32_t>(descs.size()));
            for (int i = 0; i < 16; ++i)
            {
                auto set = descMgr->GetDescriptorSet(m_bloomLayout);
                m_bloomSets.push_back(set);
            }
        }

        // Layout for Combine: binding 3,4,5,6,7 (textures) + binding 8 (cbuffer)
        {
            std::vector<gfx::GFXDescriptorSetLayoutDesc> descs = {
                {gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXGpuProgramStageFlags::Fragment, 3, 1},
                {gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXGpuProgramStageFlags::Fragment, 4, 1},
                {gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXGpuProgramStageFlags::Fragment, 5, 1},
                {gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXGpuProgramStageFlags::Fragment, 6, 1},
                {gfx::GFXDescriptorType::CombinedImageSampler, gfx::GFXGpuProgramStageFlags::Fragment, 7, 1},
                {gfx::GFXDescriptorType::ConstantBuffer,       gfx::GFXGpuProgramStageFlags::Fragment, 8, 1},
            };
            m_combineLayout = gfxApp->CreateDescriptorSetLayout(descs.data(), static_cast<uint32_t>(descs.size()));
            m_combineSet = descMgr->GetDescriptorSet(m_combineLayout);
        }

        // Create 13 independent param buffers (one per pass)
        const uint32_t PassCount = 13;
        m_bloomParamBuffers.resize(PassCount);
        gfx::GFXBufferDesc desc{};
        desc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
        desc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
        desc.BufferSize = sizeof(BloomParams);
        auto* renderThread = Application::GetGfxApp()->GetRenderThread();
        for (uint32_t i = 0; i < PassCount; ++i)
        {
            m_bloomParamBuffers[i] = renderThread->CreateBufferImmediate(desc);
        }

        auto* resMgr = Application::GetGfxApp()->GetResourceManager();

        // Bind each buffer to its corresponding set
        for (int i = 0; i < 16; ++i)
        {
            if (i < (int)m_bloomParamBuffers.size() && m_bloomSets[i])
            {
                auto* d = m_bloomSets[i]->AddDescriptor("BloomParams", 8);
                auto* buffer = resMgr->GetBuffer(m_bloomParamBuffers[i]);
                if (d && buffer) d->SetConstantBuffer(buffer);
            }
        }
        if (m_combineSet && m_bloomParamBuffers.size() > 12)
        {
            auto* d = m_combineSet->AddDescriptor("BloomParams", 8);
            auto* buffer = resMgr->GetBuffer(m_bloomParamBuffers[12]);
            if (d && buffer) d->SetConstantBuffer(buffer);
        }
    }

    void BloomPass::Destroy()
    {
        if (auto* renderThread = Application::GetGfxApp()->GetRenderThread())
        {
            for (auto& h : m_bloomParamBuffers)
            {
                if (h.IsValid()) renderThread->DestroyImmediate(h);
            }
        }
        m_bloomParamBuffers.clear();
        m_bloomSets.clear();
        m_bloomLayout.reset();
        m_combineSet.reset();
        m_combineLayout.reset();
    }

    void BloomPass::EnsureMaterial()
    {
        if (m_material)
            return;

        auto shader = AssetManager::Get()->LoadAsset<Shader>("Project/Shaders/NapBloom");
        if (!shader)
            shader = AssetManager::Get()->LoadAsset<Shader>("Shaders/NapBloom");
        if (!shader)
            return;

        m_material = Material::StaticCreate(shader);
        m_material->CreateGPUResource();
    }

    bool BloomPass::IsEnabled() const
    {
        return m_material != nullptr && m_bloomEnabled;
    }

    void BloomPass::ReadSettings(const VolumeStack& stack)
    {
        auto* settings = stack.GetComponent<BloomSettings>();
        if (settings)
        {
            m_bloomEnabled   = settings->m_enabled;
            m_bloomThreshold = settings->m_threshold;
            m_bloomIntensity = settings->m_intensity;
        }
        else
        {
            m_bloomEnabled   = true;
            m_bloomThreshold = 0.44922f;
            m_bloomIntensity = 1.0f;
        }
    }

    void BloomPass::WriteBloomParams(uint32_t idx, const Vector2f& texelSize, const Vector2f& direction, int32_t sampleMode, float threshold)
    {
        if (idx >= m_bloomParamBuffers.size() || !m_bloomParamBuffers[idx].IsValid())
            return;

        auto* buffer = Application::GetGfxApp()->GetResourceManager()->GetBuffer(m_bloomParamBuffers[idx]);
        if (!buffer) return;

        BloomParams params{};
        params.TexelSize = texelSize;
        params.Direction = direction;
        params.Threshold = threshold;
        params.Intensity = m_bloomIntensity;
        params.SampleMode = sampleMode;
        buffer->Fill(&params);
    }

    void BloomPass::SetupBloomSet(gfx::GFXDescriptorSet* set, gfx::GFXTexture2DView* srcView)
    {
        if (!set)
            return;

        auto* texDesc = set->FindByBinding(3);
        if (!texDesc) texDesc = set->AddDescriptor("PP_InColor", 3);
        if (texDesc && srcView)
            texDesc->SetTextureSampler2D(srcView);

        set->Submit();
    }

    void BloomPass::SetupCombineSet(gfx::GFXDescriptorSet* set,
                                    gfx::GFXTexture2DView* srcView,
                                    gfx::GFXTexture2DView* v0,
                                    gfx::GFXTexture2DView* v1,
                                    gfx::GFXTexture2DView* v2,
                                    gfx::GFXTexture2DView* v3)
    {
        if (!set)
            return;

        auto BindTex = [&](uint32_t binding, gfx::GFXTexture2DView* view)
        {
            auto* d = set->FindByBinding(binding);
            if (!d) d = set->AddDescriptor("Texture", binding);
            if (d && view) d->SetTextureSampler2D(view);
        };

        BindTex(3, srcView);
        BindTex(4, v0);
        BindTex(5, v1);
        BindTex(6, v2);
        BindTex(7, v3);

        set->Submit();
    }

    RGTextureHandle BloomPass::AddToGraph(RenderGraph& graph,
                                          RGTextureHandle input,
                                          RGTextureHandle output,
                                          SceneCapture2DComponent* capture2D,
                                          PerPassResources* perPass)
    {
        if (!capture2D)
            return input;

        EnsureMaterial();
        if (!m_material)
            return input;
        m_material->SubmitParameters();

        auto* camRT = capture2D->GetRenderTexture().GetPtr();
        if (!camRT)
            return input;

        uint32_t w = camRT->GetWidth();
        uint32_t h = camRT->GetHeight();
        if (w == 0 || h == 0)
            return input;

        uint32_t bloomW = 344;
        uint32_t bloomH = static_cast<uint32_t>(static_cast<float>(h) * 344.0f / static_cast<float>(w));
        if (bloomH < 1) bloomH = 1;

        uint32_t w1 = bloomW / 2;  uint32_t h1 = bloomH / 2;
        uint32_t w2 = w1 / 2;      uint32_t h2 = h1 / 2;
        uint32_t w3 = w2 / 2;      uint32_t h3 = h2 / 2;
        if (w1 < 1) w1 = 1; if (h1 < 1) h1 = 1;
        if (w2 < 1) w2 = 1; if (h2 < 1) h2 = 1;
        if (w3 < 1) w3 = 1; if (h3 < 1) h3 = 1;

        auto format = gfx::GFXTextureFormat::R16G16B16A16_SFloat;

        auto CreateRT = [&](const std::string& name, uint32_t rw, uint32_t rh) -> RGTextureHandle
        {
            RGTextureDesc desc{};
            desc.Width = rw;
            desc.Height = rh;
            desc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, format });
            return graph.CreateTransient(name, desc);
        };

        auto hBloom0  = CreateRT("Bloom0",  bloomW, bloomH);
        auto hBloom0H = CreateRT("Bloom0H", bloomW, bloomH);
        auto hBloom0V = CreateRT("Bloom0V", bloomW, bloomH);
        auto hBloom1  = CreateRT("Bloom1",  w1, h1);
        auto hBloom1H = CreateRT("Bloom1H", w1, h1);
        auto hBloom1V = CreateRT("Bloom1V", w1, h1);
        auto hBloom2  = CreateRT("Bloom2",  w2, h2);
        auto hBloom2H = CreateRT("Bloom2H", w2, h2);
        auto hBloom2V = CreateRT("Bloom2V", w2, h2);
        auto hBloom3  = CreateRT("Bloom3",  w3, h3);
        auto hBloom3H = CreateRT("Bloom3H", w3, h3);
        auto hBloom3V = CreateRT("Bloom3V", w3, h3);

        size_t bloomSetIdx = 0;
        size_t bufferIdx = 0;

        auto AddPass = [&](const std::string& passName, RGTextureHandle hSrc, RGTextureHandle hDst,
                           const std::string& shaderPassName, Vector2f texelSize,
                           Vector2f direction, int32_t sampleMode)
        {
            auto* bloomSet = m_bloomSets[bloomSetIdx++].get();
            size_t bufIdx = bufferIdx++;
            graph.AddPass(passName)
                .Read(hSrc)
                .Write(hDst, RGAttachmentDesc{
                    .colorLoadOp  = gfx::GFXRenderPassLoadOp::Clear,
                    .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                })
                .Prepare([this, shaderPassName](RGPassContext&)
                {
                    if (m_material)
                        m_material->PrepareForRendering(shaderPassName, "RENDERER_IMAGEPROCESS");
                })
                .Execute([this, hSrc, hDst, shaderPassName, texelSize, direction, sampleMode, bloomSet, bufIdx, passName]
                         (RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
                {
                    if (!m_material) return;
                    auto& binding = m_material->GetPassBinding(shaderPassName, "RENDERER_IMAGEPROCESS");
                    if (!binding.m_gpuResourcesInitialized) return;

                    auto program = binding.GetCurrentProgram();
                    if (!program) return;

                    const auto* dstRT = passCtx.Get(hDst);
                    if (!dstRT) return;
                    auto* dstFBO = dstRT->GetFrameBufferObject().get();
                    if (!dstFBO) return;

                    const auto* srcRT = passCtx.Get(hSrc);
                    gfx::GFXTexture2DView* srcView = nullptr;
                    if (srcRT)
                    {
                        auto view = srcRT->GetRenderTarget0();
                        if (view) srcView = view.get();
                    }

                    float threshold = (passName == "Bloom_Downsample0") ? m_bloomThreshold : 0.0f;
                    WriteBloomParams((uint32_t)bufIdx, texelSize, direction, sampleMode, threshold);
                    SetupBloomSet(bloomSet, srcView);

                    auto* gfxApp = cmdBuffer.GetApplication();
                    auto* pipelineMgr = gfxApp->GetGraphicsPipelineManager();

                    array_list<gfx::GFXDescriptorSetLayout_sp> descLayouts;
                    descLayouts.push_back(binding.m_descriptorSetLayout);
                    descLayouts.push_back(m_bloomLayout);

                    gfx::GFXGraphicsPipelineStateParams psoParams{};
                    psoParams.DepthTestEnable  = false;
                    psoParams.DepthWriteEnable = false;
                    psoParams.CullMode         = gfx::GFXCullMode::None;

                    auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(
                        program->GetGpuPrograms(), psoParams, descLayouts,
                        dstFBO->GetRenderTargetDesc(), {});
                    if (!gfxPipeline) return;

                    cmdBuffer.CmdSetViewport(0, 0, (float)dstFBO->GetWidth(), (float)dstFBO->GetHeight());
                    cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());
                    cmdBuffer.CmdSetCullMode(gfx::GFXCullMode::None);

                    array_list<gfx::GFXDescriptorSet*> descSets;
                    descSets.push_back(binding.m_descriptorSet.get());
                    descSets.push_back(bloomSet);
                    cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get());

                    cmdBuffer.CmdDraw(3);
                });
        };

        // Layer 0: downsample + threshold
        AddPass("Bloom_Downsample0", input, hBloom0, "BloomDownsample",
                Vector2f(1.0f / w, 1.0f / h), Vector2f(0, 0), 0);

        // Layer 0: blur horizontal (6 samples)
        AddPass("Bloom_BlurH0", hBloom0, hBloom0H, "BloomGaussFilter",
                Vector2f(1.0f / bloomW, 1.0f / bloomH), Vector2f(1, 0), 0);

        // Layer 0: blur vertical (6 samples)
        AddPass("Bloom_BlurV0", hBloom0H, hBloom0V, "BloomGaussFilter",
                Vector2f(1.0f / bloomW, 1.0f / bloomH), Vector2f(0, 1), 0);

        // Layer 1: downsample
        AddPass("Bloom_Downsample1", hBloom0V, hBloom1, "BloomDownsample",
                Vector2f(1.0f / bloomW, 1.0f / bloomH), Vector2f(0, 0), 0);

        // Layer 1: blur horizontal (9 samples)
        AddPass("Bloom_BlurH1", hBloom1, hBloom1H, "BloomGaussFilter",
                Vector2f(1.0f / w1, 1.0f / h1), Vector2f(1, 0), 1);

        // Layer 1: blur vertical (9 samples)
        AddPass("Bloom_BlurV1", hBloom1H, hBloom1V, "BloomGaussFilter",
                Vector2f(1.0f / w1, 1.0f / h1), Vector2f(0, 1), 1);

        // Layer 2: downsample
        AddPass("Bloom_Downsample2", hBloom1V, hBloom2, "BloomDownsample",
                Vector2f(1.0f / w1, 1.0f / h1), Vector2f(0, 0), 0);

        // Layer 2: blur horizontal (16 samples)
        AddPass("Bloom_BlurH2", hBloom2, hBloom2H, "BloomGaussFilter",
                Vector2f(1.0f / w2, 1.0f / h2), Vector2f(1, 0), 2);

        // Layer 2: blur vertical (16 samples)
        AddPass("Bloom_BlurV2", hBloom2H, hBloom2V, "BloomGaussFilter",
                Vector2f(1.0f / w2, 1.0f / h2), Vector2f(0, 1), 2);

        // Layer 3: downsample
        AddPass("Bloom_Downsample3", hBloom2V, hBloom3, "BloomDownsample",
                Vector2f(1.0f / w2, 1.0f / h2), Vector2f(0, 0), 0);

        // Layer 3: blur horizontal (20 samples)
        AddPass("Bloom_BlurH3", hBloom3, hBloom3H, "BloomGaussFilter",
                Vector2f(1.0f / w3, 1.0f / h3), Vector2f(1, 0), 3);

        // Layer 3: blur vertical (20 samples)
        AddPass("Bloom_BlurV3", hBloom3H, hBloom3V, "BloomGaussFilter",
                Vector2f(1.0f / w3, 1.0f / h3), Vector2f(0, 1), 3);

        // Combine: original + bloom layers -> output
        graph.AddPass("Bloom_Combine")
            .Read(input)
            .Read(hBloom0V)
            .Read(hBloom1V)
            .Read(hBloom2V)
            .Read(hBloom3V)
            .Write(output, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Clear,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
            })
            .Prepare([this](RGPassContext&)
            {
                if (m_material)
                    m_material->PrepareForRendering("BloomCombine", "RENDERER_IMAGEPROCESS");
            })
            .Execute([this, input, hBloom0V, hBloom1V, hBloom2V, hBloom3V, output]
                     (RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                if (!m_material) return;
                auto& binding = m_material->GetPassBinding("BloomCombine", "RENDERER_IMAGEPROCESS");
                if (!binding.m_gpuResourcesInitialized) return;

                auto program = binding.GetCurrentProgram();
                if (!program) return;

                const auto* dstRT = passCtx.Get(output);
                if (!dstRT) return;
                auto* dstFBO = dstRT->GetFrameBufferObject().get();
                if (!dstFBO) return;

                auto GetView = [&](RGTextureHandle h) -> gfx::GFXTexture2DView*
                {
                    const auto* rt = passCtx.Get(h);
                    if (rt)
                    {
                        auto view = rt->GetRenderTarget0();
                        if (view) return view.get();
                    }
                    return nullptr;
                };

                WriteBloomParams(12, Vector2f(0, 0), Vector2f(0, 0), 0, 0.0f);
                SetupCombineSet(m_combineSet.get(),
                                GetView(input), GetView(hBloom0V),
                                GetView(hBloom1V), GetView(hBloom2V),
                                GetView(hBloom3V));

                auto* gfxApp = cmdBuffer.GetApplication();
                auto* pipelineMgr = gfxApp->GetGraphicsPipelineManager();
                array_list<gfx::GFXDescriptorSetLayout_sp> descLayouts;
                descLayouts.push_back(binding.m_descriptorSetLayout);
                descLayouts.push_back(m_combineLayout);

                gfx::GFXGraphicsPipelineStateParams psoParams{};
                psoParams.DepthTestEnable  = false;
                psoParams.DepthWriteEnable = false;
                psoParams.CullMode         = gfx::GFXCullMode::None;

                auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(
                    program->GetGpuPrograms(), psoParams, descLayouts,
                    dstFBO->GetRenderTargetDesc(), {});
                if (!gfxPipeline) return;

                cmdBuffer.CmdSetViewport(0, 0, (float)dstFBO->GetWidth(), (float)dstFBO->GetHeight());
                cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());
                cmdBuffer.CmdSetCullMode(gfx::GFXCullMode::None);

                array_list<gfx::GFXDescriptorSet*> descSets;
                descSets.push_back(binding.m_descriptorSet.get());
                descSets.push_back(m_combineSet.get());
                cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get());

                cmdBuffer.CmdDraw(3);
            });

        return output;
    }
}
