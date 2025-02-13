#pragma once
#include "CurveLinearColor.h"
#include "Texture2D.h"

namespace pulsar
{
    class CurveLinearColorAtlas : public Texture2D
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CurveLinearColorAtlas, Texture2D);
    public:
        CurveLinearColorAtlas();
        void Generate();
    protected:

        CORELIB_REFL_DECL_FIELD(m_textureWidth, new RangePropertyAttribute(0, 2048))
        int m_textureWidth;

        CORELIB_REFL_DECL_FIELD(m_curves);
        List_sp<RCPtr<CurveLinearColor>> m_curves;
    };
}