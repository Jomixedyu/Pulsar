#include "Components/SceneCaptureComponent.h"
#include "World.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/BuiltinAsset.h>
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Subsystems/PostProcessSubsystem.h>


namespace pulsar
{

    static void BuildViewPipelineRenderData(const RCPtr<ViewPipelineSettings>& viewPipeline, ViewPipelineRenderData& outData)
    {
        if (viewPipeline)
        {
            viewPipeline->BuildRenderData(outData);
            return;
        }

        if (auto defaultPipeline = AssetManager::Get()->LoadAsset<ViewPipelineSettings>(BuiltinAsset::ViewPipeline_DefaultScene))
        {
            defaultPipeline->BuildRenderData(outData);
        }
    }

    SceneCaptureComponent::SceneCaptureComponent()
    {
    }

    SPtr<rendering::RenderProxy> SceneCaptureComponent::CreateRenderProxy()
    {
        auto view = mksptr(new SceneView());
        BuildViewPipelineRenderData(m_viewPipelineSettings, view->Data.ViewPipeline);
        m_sceneView = view;
        return view;
    }

    void SceneCaptureComponent::ResolveRenderStateDirty()
    {
        if (!m_sceneView)
            return;

        const bool needsViewData = m_renderDirtyTransform || m_renderDirtyCamera || m_renderDirtyRenderTarget;
        if (!needsViewData && !m_renderDirtyRenderFeature && !m_renderDirtyPostProcess)
            return;

        SceneViewData data = m_sceneView->Data;
        if (needsViewData)
        {
            if (!ExtractViewData(data))
                return;
        }

        if (m_renderDirtyRenderFeature)
            BuildViewPipelineRenderData(m_viewPipelineSettings, data.ViewPipeline);

        if (m_renderDirtyPostProcess)
        {
            if (auto* ppSub = GetWorld()->GetSubsystem<PostProcessSubsystem>())
            {
                data.PostProcessStack = ppSub->QuerySettings(data.CameraPosition);
                data.PostProcessMaterials = ppSub->QueryPostProcessMaterials(data.CameraPosition);
            }
        }

        m_renderDirtyTransform = false;
        m_renderDirtyCamera = false;
        m_renderDirtyRenderTarget = false;
        m_renderDirtyRenderFeature = false;
        m_renderDirtyPostProcess = false;

        GetWorld()->UpdateSceneView(m_sceneView, std::move(data));
    }

    void SceneCaptureComponent::BeginComponent()
    {
        base::BeginComponent();
        RebuildObserver();
        GetWorld()->GetCaptureManager().Add(this);
        // Push an initial snapshot to the freshly-created view proxy.
        MarkRenderStateDirty();
    }
    void SceneCaptureComponent::EndComponent()
    {
        m_sceneView.reset();
        base::EndComponent();
        GetWorld()->GetCaptureManager().Remove(this);
    }



    void SceneCaptureComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == NAMEOF(m_viewPipelineSettings))
        {
            RebuildObserver();
            m_renderDirtyRenderFeature = true;
            MarkRenderStateDirty();
        }
    }

    void SceneCaptureComponent::GetSubscribeObserverHandles(array_list<ObjectHandle>& out)
    {
        base::GetSubscribeObserverHandles(out);
        if (m_viewPipelineSettings)
            out.push_back(m_viewPipelineSettings.GetHandle());
        else if (auto defaultPipeline = AssetManager::Get()->LoadAsset<ViewPipelineSettings>(BuiltinAsset::ViewPipeline_DefaultScene))
            out.push_back(defaultPipeline.GetHandle());
    }

    void SceneCaptureComponent::OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg)
    {
        base::OnNotifyObserver(inDependency, msg);
        if (EnumHasFlag(msg, DependencyObjectState::Modified))
        {
            m_renderDirtyRenderFeature = true;
            MarkRenderStateDirty();
        }
    }

}
