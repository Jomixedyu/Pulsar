#include "SceneSettings.h"

namespace pulsar
{
    SceneSettings::SceneSettings()
        : m_renderSettings(mksptr(new RenderSettings()))
    {
    }
}