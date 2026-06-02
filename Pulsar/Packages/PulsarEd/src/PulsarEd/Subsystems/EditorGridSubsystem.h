#pragma once
#include "Assembly.h"
#include <Pulsar/Subsystems/WorldSubsystem.h>

namespace pulsared
{

    class EditorGridSubsystem : public WorldSubsystem
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::EditorGridSubsystem, WorldSubsystem);
    public:

        enum Mode { e3D, e2D };

        void SetGridMode(Mode mode);

        void SetVisible(bool visible);
        bool IsVisible() const { return m_visible; }
        void OnInitialized() override;

    private:

        ObjectPtr<Node> m_grid3d;
        ObjectPtr<Node> m_grid2d;

        bool m_visible = true;
        Mode m_mode{};
    };
    SUBSYSTEM_DECL(EditorGridSubsystem);
} // namespace pulsared