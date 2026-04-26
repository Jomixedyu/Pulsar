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

        size_t GetCurveCount() const override
        {
            return m_curvesData->size();
        }
        const SPtr<CurveData>& GetCurveData(size_t index) const
        {
            return m_curvesData->at(index);
        }

        void SetColorsKey(int index, std::array<CurveKey, 4> keys);
        std::array<CurveKey, 4> GetColorsKey(int index) const;

        void InsertColorKey();


        Color4f SampleColor(float t) const;

        void PostEditChange(FieldInfo* info) override;

    protected:
        CORELIB_REFL_DECL_FIELD(m_curvesData);
        List_sp<SPtr<CurveData>> m_curvesData;
    };

} // namespace pulsar