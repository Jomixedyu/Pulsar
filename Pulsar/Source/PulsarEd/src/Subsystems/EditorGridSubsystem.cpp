#include "Subsystems/EditorGridSubsystem.h"

#include "Components/Grid2DComponent.h"
#include "Components/Grid3DComponent.h"
#include "Pulsar/Scene.h"

namespace pulsared
{
    void EditorGridSubsystem::SetGridMode(Mode mode)
    {
        m_mode = mode;
        if (m_visible)
        {
            if (mode == e2D)
            {
                m_grid3d->SetIsActiveSelf(false);
                m_grid2d->SetIsActiveSelf(true);
            }
            else if (mode == e3D)
            {
                m_grid3d->SetIsActiveSelf(true);
                m_grid2d->SetIsActiveSelf(false);
            }
        }
    }

    void EditorGridSubsystem::SetVisible(bool visible)
    {
        m_visible = visible;
        if (visible)
        {
            SetGridMode(m_mode);
        }
        else
        {
            m_grid3d->SetIsActiveSelf(false);
            m_grid2d->SetIsActiveSelf(false);
        }

    }

    void EditorGridSubsystem::OnInitialized()
    {
        base::OnInitialized();
        m_grid3d = m_world->GetResidentScene()->NewNode("Grid3D", {}, OF_NoPack);
        m_grid3d->AddComponent<Grid3DComponent>();
        m_grid3d->SetIsActiveSelf(false);

        m_grid2d = m_world->GetResidentScene()->NewNode("Grid2D", {}, OF_NoPack);
        m_grid2d->AddComponent<Grid2DComponent>();
        m_grid2d->SetIsActiveSelf(false);

        SetVisible(true);
    }
} // namespace pulsared