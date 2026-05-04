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
            return 4;
        }
        CurveKeyCollection GetCurveData(size_t index) const;

        int GetKeyCount() const { return (int)m_R.GetKeyCount(); }
        int GetKeyCountA() const { return (int)m_A.GetKeyCount(); }

        void SetKeyR(int index, CurveKey r);
        void SetKeyG(int index, CurveKey g);
        void SetKeyB(int index, CurveKey b);
        void SetKeyA(int index, CurveKey a);

        void SetAllColorKey(int index, CurveKey all);
        void ClearKeys();
        void AddKeyR(CurveKey key);
        void AddKeyG(CurveKey key);
        void AddKeyB(CurveKey key);
        void AddKeyA(CurveKey key);

        CurveKey GetKeyR(int index) const;
        CurveKey GetKeyG(int index) const;
        CurveKey GetKeyB(int index) const;
        CurveKey GetKeyA(int index) const;

        std::array<CurveKey, 4> GetColorsKey(int index) const;
        void SetColorsKey(int index, const std::array<CurveKey, 4>& colors);

        void AddKey(CurveKey key);

        Color4f SampleColor(float t) const;

        void PostEditChange(FieldInfo* info) override;

    protected:

        CurveKeyCollection m_R;
        CurveKeyCollection m_G;
        CurveKeyCollection m_B;
        CurveKeyCollection m_A;
    };

} // namespace pulsar