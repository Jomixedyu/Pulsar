#pragma once
#include <Pulsar/AssetObject.h>

namespace pulsar
{
    enum class CurveInterpMode
    {
        Linear,
        Constant,
        Cubic
    };

    enum class CurveTangentMode
    {
        Auto,
        User,
        Break,
    };

    enum class CurveTangentWeightMode
    {
        None,
        Arrive,
        Leave,
        Both
    };
    enum class CurveExtrapolationMode
    {
        Cycle,
        CycleWithOffset,
        Oscillate,
        Linear,
        Constant
    };

    struct CurveKey
    {
        CurveInterpMode InterpMode;
        CurveTangentMode TangentMode;
        CurveTangentWeightMode TangentWeightMode;

        float Time;
        float Value;
        float ArriveTangent;
        float ArriveTangentWeight;
        float LeaveTangent;
        float LeaveTangentWeight;

        bool operator==(const CurveKey&) const = default;
    };

    class BoxingCurveKey : public BoxingObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxingCurveKey, BoxingObject);
    public:
        using unboxing_type = CurveKey;
        BoxingCurveKey() {}
        BoxingCurveKey(CurveKey key)
        {
            Time = key.Time;
            Value = key.Value;
        }
        CurveKey get_unboxing_value()
        {
            CurveKey key{};
            key.Time = Time;
            key.Value = Value;
            return key;
        }

        CORELIB_REFL_DECL_FIELD(Time);
        float Time{};
        CORELIB_REFL_DECL_FIELD(Value);
        float Value{};
    };
}

CORELIB_DECL_BOXING(pulsar::CurveKey, pulsar::BoxingCurveKey);

namespace pulsar
{
    class CurveData final : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CurveData, Object);
    public:
        CurveData() : DefaultValue(0)
        {
            init_sptr_member(Keys);
        }
        float Sample(float X, float InDefaultValue = 0.0f) const;

        size_t GetKeyCount() const { return Keys->size(); }
        CurveKey GetKey(size_t index) const { return Keys->at(index); }
        void SetKey(size_t index, CurveKey key) { Keys->at(index) = key; }
        void AddKey(CurveKey key) { Keys->push_back(key); }

    protected:
        CORELIB_REFL_DECL_FIELD(Keys);
        SPtr<List<CurveKey>> Keys;

        CORELIB_REFL_DECL_FIELD(DefaultValue);
        float DefaultValue;
    };

    class Curve : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Curve, AssetObject);
    public:
        virtual size_t GetCurveCount() const = 0;

    };
}