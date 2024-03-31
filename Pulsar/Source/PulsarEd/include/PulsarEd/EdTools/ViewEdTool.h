#pragma once
#include "EdTool.h"

namespace pulsared
{

    struct MouseEventData
    {
        int ButtonId;
        bool InRegion;
        Vector2f InRegionPosition;
        Vector2f Position;
    };
    class ViewEdTool : public EdTool
    {
        using base = EdTool;
    public:
        void Tick(float dt) override;

        void Begin() override;

    protected:
        virtual void OnMouseDown(const MouseEventData& e);
        virtual void OnMouseUp(const MouseEventData& e);
    public:
        bool m_altPressed{};
        bool m_leftMousePressed{};
        bool m_middleMousePressed{};
        bool m_rightMousePressed{};
        bool m_enabledRotate{true};
        Vector2f m_latestMousePos{};
        float m_scaleSpeed = 0.1f;
    };

} // namespace pulsared
