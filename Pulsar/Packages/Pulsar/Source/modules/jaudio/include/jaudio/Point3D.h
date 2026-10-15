#pragma once
#include "Define.h"
#include <cstdint>

namespace jaudio
{
    class JAUDIO_API Point3D
    {
    public:
        void SetPosition(float x, float y, float z)
        {
            m_x = x;  m_y = y; m_z = z;
            OnPositionChanged();
        }
        void GetPosition(float* x, float* y, float* z) const
        {
            *x = m_x; *y = m_y; *z = m_z;
        }
    protected:
        virtual void OnPositionChanged() {}
    protected:
        float m_x = 0;
        float m_y = 0;
        float m_z = 0;
    };
}