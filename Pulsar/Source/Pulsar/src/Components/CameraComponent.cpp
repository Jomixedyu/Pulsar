#include "Components/CameraComponent.h"

#include "AssetManager.h"
#include "Logger.h"
#include "Scene.h"
#include <Pulsar/AppInstance.h>
#include <Pulsar/Application.h>
#include <Pulsar/Node.h>
#include <Pulsar/TransformUtil.h>

namespace pulsar
{

    CameraComponent::CameraComponent() = default;
    CameraComponent::~CameraComponent() = default;


    void CameraComponent::Render()
    {
    }


    void CameraComponent::Render(array_list<RenderCapturePassInfo*>& passes)
    {
        base::Render(passes);


    }

    void CameraComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        auto name = info->GetName();
        if (name == NAMEOF(m_backgroundColor))
        {
            UpdateRTBackgroundColor();
        }
        else if(name == NAMEOF(m_renderingPath))
        {
            auto width = GetRenderTexture()->GetWidth();
            auto height = GetRenderTexture()->GetHeight();
            ResizeManagedRenderTexture(width, height);
        }
        else if(name == NAMEOF(m_postProcessMaterials))
        {
            for (auto& item : *m_postProcessMaterials)
            {
                TryLoadAssetRCPtr(item);
                if (item)
                {
                    item->CreateGPUResource();
                }
            }
        }
        UpdateCBuffer();
    }

    void CameraComponent::ResizeManagedRenderTexture(int width, int height)
    {
        if (!m_beginning)
            return;
        if (!m_managedRT)
            return;

        if (m_renderTarget)
        {
            DestroyObject(m_renderTarget);
        }
        DestroyObject(m_postprocessRtA);
        DestroyObject(m_postprocessRtB);

        auto rtname = GetNode()->GetName() + "_CamRT";

        array_list<RenderTargetInfo> formats;

        if (m_renderingPath == RenderingPathMode::Deferred)
        {
            // GBUFFER_0: Albedo(rgb), genericAO(a)
            formats.push_back({ gfx::GFXTextureTargetType::ColorTarget, gfx::GFXTextureFormat::R8G8B8A8_UNorm});
            // GBUFFER_1: WorldNormal(rgb), ShadingModel(a)
            formats.push_back({ gfx::GFXTextureTargetType::ColorTarget, gfx::GFXTextureFormat::R8G8B8A8_UNorm});
            // GBUFFER_2: M/R(rg), Specular(b), _(a)
            formats.push_back({ gfx::GFXTextureTargetType::ColorTarget, gfx::GFXTextureFormat::R8G8B8A8_UNorm});
            // GBUFFER_3: WorldTangent(rgb), Aniso(a)
            formats.push_back({ gfx::GFXTextureTargetType::ColorTarget, gfx::GFXTextureFormat::R8G8B8A8_UNorm});
            // GBUFFER_4: ShadowColor(rgb), NdotL(a)
            formats.push_back({ gfx::GFXTextureTargetType::ColorTarget, gfx::GFXTextureFormat::R8G8B8A8_UNorm});
        }
        else if (m_renderingPath == RenderingPathMode::Forward)
        {
            assert((false, "not impl"));
        }
        else
        {
            assert((false, "not impl"));
        }
        formats.push_back({
            .TargetType = gfx::GFXTextureTargetType::DepthStencilTarget, .Format = gfx::GFXTextureFormat::D32_SFloat_S8_UInt});

        m_renderTarget = RenderTexture::StaticCreate(index_string{rtname}, width, height, formats);


        array_list<RenderTargetInfo> ppTargetInfo;
        ppTargetInfo.push_back({gfx::GFXTextureTargetType::ColorTarget, gfx::GFXTextureFormat::R16G16B16A16_SFloat});
        m_postprocessRtA = RenderTexture::StaticCreate(index_string{rtname}, width, height, ppTargetInfo);
        m_postprocessRtB = RenderTexture::StaticCreate(index_string{rtname}, width, height, ppTargetInfo);

        array_list<RenderTargetInfo> sceneTargetInfo;
        sceneTargetInfo.push_back({gfx::GFXTextureTargetType::ColorTarget, gfx::GFXTextureFormat::R16G16B16A16_SFloat});
        m_sceneColor = RenderTexture::StaticCreate(index_string{rtname}, width, height, ppTargetInfo);

        gfx::GFXDescriptorSetLayoutInfo ppDescLayouts[2]{
            {gfx::GFXDescriptorType::CombinedImageSampler,
             gfx::GFXShaderStageFlags::VertexFragment,
             0, 2},
            {gfx::GFXDescriptorType::CombinedImageSampler,
             gfx::GFXShaderStageFlags::VertexFragment,
             1, 2},
        };

        auto layout = Application::GetGfxApp()->CreateDescriptorSetLayout(ppDescLayouts, 2);
        m_postprocessDescA = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(layout);
        m_postprocessDescA->AddDescriptor("Color", 0)->SetTextureSampler2D(m_postprocessRtB->GetGfxRenderTarget0().get());
        m_postprocessDescA->Submit();
        m_postprocessDescB = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(layout);
        m_postprocessDescB->AddDescriptor("Color", 0)->SetTextureSampler2D(m_postprocessRtA->GetGfxRenderTarget0().get());
        m_postprocessDescB->Submit();

        UpdateRT();
        BeginRT();


    }






    void CameraComponent::UpdateRT()
    {
        UpdateRTBackgroundColor();
    }

    void CameraComponent::SetRenderTexture(const RCPtr<RenderTexture>& value, bool managed)
    {
        if (value)
        {
            if (m_managedRT)
            {
                DestroyObject(m_renderTarget);
                m_managedRT = false;
            }
            m_renderTarget = value;
            m_managedRT = managed;
        }
        else // null
        {
            if (!m_managedRT)
            {
                m_renderTarget = value;
                m_managedRT = managed;
            }
        }

        UpdateRT();
        BeginRT();
    }



    Ray CameraComponent::ScreenPointToRay(Vector2f mousePosition) const
    {
        auto [width, height] = GetRenderTexture()->GetSize2df();

        // convert to NDC
        auto x = 2.f * mousePosition.x / width - 1.f;
        auto y = -2.f * mousePosition.y / height + 1.f;

        auto invMat = GetInvViewProjectionMat();

        auto near = invMat * Vector4f {x, y, 0.f, 1.f};
        auto far  = invMat * Vector4f {x, y, 1.f, 1.f};

        far /= far.w;

        Ray ray{};
        ray.Origin = near.xyz();
        ray.Direction = Normalize(far.xyz());
        return ray;
    }

    void CameraComponent::BeginRT()
    {
        if (m_beginning && m_renderTarget)
        {
            auto& refData = m_renderTarget->GetGfxFrameBufferObject()->RefData;
            refData.clear();
            refData.push_back(m_cameraDescriptorSet);

            UpdateCBuffer();
        }
    }

    void CameraComponent::BeginComponent()
    {
        base::BeginComponent();

        if (!m_renderTarget)
        {
            m_managedRT = true;
            ResizeManagedRenderTexture(1, 1);
        }

        GetNode()->GetRuntimeOwnerScene()->GetWorld()->GetCameraManager().AddCamera(THIS_REF);

        BeginRT();
    }

    void CameraComponent::EndComponent()
    {
        base::EndComponent();
        GetNode()->GetRuntimeOwnerScene()->GetWorld()->GetCameraManager().RemoveCamera(THIS_REF);
        if (m_managedRT && m_renderTarget)
        {
            DestroyObject(m_renderTarget);
        }
        if (m_managedRT)
        {

        }
        m_camDescriptorLayout.reset();
        m_cameraDescriptorSet.reset();
        m_cameraDataBuffer.reset();
    }
} // namespace pulsar
