#include "CustomPostProcessChain.h"
#include "CustomPostProcessPass.h"
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Components/SceneCaptureComponent.h>
#include <Pulsar/Subsystems/PostProcessSubsystem.h>
#include <Pulsar/World.h>
#include <Pulsar/Node.h>

namespace pulsar
{
    CustomPostProcessChain::CustomPostProcessChain() = default;
    CustomPostProcessChain::~CustomPostProcessChain() = default;

    void CustomPostProcessChain::OnSetup(const RenderCaptureContext& ctx)
    {
        m_materials.clear();
        auto* cam = dynamic_cast<CameraComponent*>(ctx.capture);
        if (!cam || !ctx.world)
            return;

        if (auto* ppSub = ctx.world->GetSubsystem<PostProcessSubsystem>())
        {
            auto camPos = cam->GetNode()->GetTransform()->GetWorldPosition();
            m_materials = ppSub->QueryPostProcessMaterials(camPos);
        }
    }

    bool CustomPostProcessChain::IsEnabled() const
    {
        return !m_materials.empty();
    }

    RGTextureHandle CustomPostProcessChain::AddToGraph(RenderGraph& graph,
                                                       RGTextureHandle input,
                                                       RGTextureHandle output,
                                                       CameraComponent* cam,
                                                       PerPassResources* perPass)
    {
        if (m_materials.empty())
            return input;

        // Ensure pool size
        if (m_passPool.size() < m_materials.size())
        {
            size_t oldSize = m_passPool.size();
            m_passPool.resize(m_materials.size());
            for (size_t i = oldSize; i < m_passPool.size(); ++i)
            {
                m_passPool[i] = std::make_unique<CustomPostProcessPass>();
                m_passPool[i]->Initialize(perPass);
            }
        }

        RGTextureHandle curSrc = input;
        RGTextureHandle curDst = output;

        for (size_t i = 0; i < m_materials.size(); ++i)
        {
            auto* mat = m_materials[i].GetPtr();
            if (!mat)
                continue;

            auto& pass = m_passPool[i];
            pass->SetMaterial(m_materials[i]);
            pass->SetPassName(std::string("PostProcess_PPCompMat_") + std::to_string(i));
            curDst = pass->AddToGraph(graph, curSrc, curDst, cam, perPass);
            std::swap(curSrc, curDst);
        }

        return curSrc;
    }
}
