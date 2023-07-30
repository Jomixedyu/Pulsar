#pragma once
#include <gfx/GFXApplication.h>
#include <gfx/GFXBuffer.h>
#include "EngineMath.h"

namespace pulsar::rendering
{
    struct MeshSection
    {

    };

    struct MeshBatch
    {

    };

    class IDrawInterface
    {
    public:

    };

    class RenderObject
    {
    public:
        void SetTransform(const Matrix4f& localToWorld)
        {
            m_localToWorld = localToWorld;
            //m_isLocalToWorldDeterminantNegative = localToWorld.Determinant() < 0;
            m_isLocalToWorldDeterminantNegative = false;
            UpdateConstantBuffer();
        }
        void UpdateConstantBuffer()
        {

        }
    protected:
        Matrix4f  m_localToWorld;
        bool      m_isLocalToWorldDeterminantNegative;

    };
}