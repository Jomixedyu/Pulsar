#pragma once
#include "EdTool.h"

namespace pulsared
{

    struct MouseEventData
    {
        int ButtonId;
        bool InRegion;
        bool AltFunction;
        bool CtrlFunction;
        bool ShiftFunction;
        Vector2f InRegionPosition;
        Vector2f Position;
        bool FunctionKey() const
        {
            return AltFunction || CtrlFunction || ShiftFunction;
        }
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
        virtual void OnDragUpdate(const MouseEventData& e);

        void GetViewportSize(Vector2f& pos, Vector2f& size) const;
    public:
        bool m_altPressed{};
        bool m_leftMousePressed{};
        bool m_middleMousePressed{};
        bool m_rightMousePressed{};
        Vector2f m_latestMousePos{};
        float m_scaleSpeed = 0.1f;
    };

} // namespace pulsared
