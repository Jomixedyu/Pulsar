#pragma once
#include <Pulsar/Assembly.h>
#include <Pulsar/Assets/CubeMap.h>
#include <Pulsar/Settings/RenderSettings.h>

namespace pulsar
{
    class SceneSettings : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SceneSettings, Object);

    public:
        SceneSettings();

        RenderSettings* GetRenderSettings() { return m_renderSettings.get(); }

        CORELIB_REFL_DECL_FIELD(m_overrideRenderSettings);
        bool m_overrideRenderSettings = false;

        CORELIB_REFL_DECL_FIELD(m_renderSettings);
        SPtr<RenderSettings> m_renderSettings;

        CORELIB_REFL_DECL_FIELD(m_cubemap);
        CubeMapAsset_ref m_cubemap;
    };
}