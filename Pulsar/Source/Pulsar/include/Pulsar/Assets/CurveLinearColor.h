#pragma once
#include "Curve.h"

namespace pulsar
{
    class CurveLinearColor : public Curve
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CurveLinearColor, Curve);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute);
    public:
        CurveLinearColor();
        void Serialize(AssetSerializer* s) override;

        size_t GetCurveCount() const override { return m_curvesData->size(); }
        const SPtr<CurveData>& GetCurveData(size_t index) const { return m_curvesData->at(index); }

        Color4f SampleColor(float t) const;

    protected:
        CORELIB_REFL_DECL_FIELD(m_curvesData);
        List_sp<SPtr<CurveData>> m_curvesData;
    };
}