#pragma once
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeatureSettings.h>

namespace pulsar
{
    class GizmoOverlayRenderFeatureSettings : public RenderFeatureSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::GizmoOverlayRenderFeatureSettings, RenderFeatureSettings);
    public:
        RenderFeatureFactory BuildRenderFeatureFactory() const override;
    };
}