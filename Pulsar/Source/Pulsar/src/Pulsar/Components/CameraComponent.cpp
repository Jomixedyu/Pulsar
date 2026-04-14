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
        if (!m_isBegun)
            return;
        if (!m_managedRT)
            return;

        if (m_renderTarget)
        {
            DestroyObject(m_renderTarget);
        }
        // Task 7.3: m_postprocessRtA/B removed (now managed by TransientRTPool via RenderGraph)

        auto rtname = GetNode()->GetName() + "_CamRT";

        array_list<RenderTargetInfo> formats;

        formats.push_back({ gfx::GFXTextureTargetType::ColorTarget, gfx::GFXTextureFormat::R8G8B8A8_UNorm});

        formats.push_back({
            .TargetType = gfx::GFXTextureTargetType::DepthStencilTarget, .Format = gfx::GFXTextureFormat::D32_SFloat_S8_UInt});

        m_renderTarget = RenderTexture::StaticCreate(index_string{rtname}, width, height, formats);


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

        near /= near.w;
        far  /= far.w;

        Ray ray{};
        ray.Origin    = near.xyz();
        ray.Direction = Normalize(far.xyz() - near.xyz());
        return ray;
    }

    void CameraComponent::BeginRT()
    {
        if (m_isBegun && m_renderTarget)
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

        GetNode()->GetRuntimeWorld()->GetCameraManager().AddCamera(self_ptr());

        BeginRT();
    }

    void CameraComponent::EndComponent()
    {
        base::EndComponent();
        GetNode()->GetRuntimeWorld()->GetCameraManager().RemoveCamera(self_ptr());
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
