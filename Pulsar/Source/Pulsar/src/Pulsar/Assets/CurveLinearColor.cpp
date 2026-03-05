#include "Pulsar/Assets/CurveLinearColor.h"

#include "CoreLib.Serialization/JsonSerializer.h"

namespace pulsar
{

    CurveLinearColor::CurveLinearColor()
    {
        init_sptr_member(m_curvesData);
        for (int i = 0; i < 4; ++i)
        {
            auto& curve = m_curvesData->emplace_back(mksptr(new CurveData));
            CurveKey key0{};
            key0.Time = 0;
            key0.Value = 0;
            curve->AddKey(key0);

            CurveKey key1{};
            key1.Time = 1;
            key1.Value = 1;
            curve->AddKey(key1);
        }
    }
    void CurveLinearColor::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (s->IsWrite)
        {
            auto curvesArray = s->Object->New(ser::VarientType::Array);

            for (auto& curve : *m_curvesData)
            {
                auto curveObj = curvesArray->New(ser::VarientType::Object);

                auto js = ser::JsonSerializer::Serialize(curve.get(), {});
                curveObj->AssignParse(js);

                curvesArray->Push(curveObj);
            }

            s->Object->Add("Curves", curvesArray);
        }
        else
        {
            m_curvesData->clear();
            auto curvesArray = s->Object->At("Curves");
            for (int i = 0; i < curvesArray->GetCount(); ++i)
            {
                auto curveObj = curvesArray->At(i);
                auto curveData = ser::JsonSerializer::Deserialize<CurveData>(curveObj->ToString());
                m_curvesData->push_back(curveData);
            }
        }
    }

    void CurveLinearColor::SetColorsKey(int index, std::array<CurveKey, 4> keys)
    {
        m_curvesData->at(0)->SetKey(index, keys[0]);
        m_curvesData->at(1)->SetKey(index, keys[1]);
        m_curvesData->at(2)->SetKey(index, keys[2]);
        m_curvesData->at(3)->SetKey(index, keys[3]);
    }

    std::array<CurveKey, 4> CurveLinearColor::GetColorsKey(int index) const
    {
        return std::array<CurveKey, 4>{
            m_curvesData->at(0)->GetKey(index),
            m_curvesData->at(1)->GetKey(index),
            m_curvesData->at(2)->GetKey(index),
            m_curvesData->at(3)->GetKey(index),
        };
    }
    void CurveLinearColor::InsertColorKey()
    {
        m_curvesData->at(0)->AddKey({});
        m_curvesData->at(1)->AddKey({});
        m_curvesData->at(2)->AddKey({});
        m_curvesData->at(3)->AddKey({});
    }

    Color4f CurveLinearColor::SampleColor(float t) const
    {
        Color4f color;
        color.r = m_curvesData->at(0)->Sample(t);
        color.g = m_curvesData->at(1)->Sample(t);
        color.b = m_curvesData->at(2)->Sample(t);
        color.a = m_curvesData->at(3)->Sample(t);

        return color;
    }

    void CurveLinearColor::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        RuntimeObjectManager::NotifyDependencySource(GetObjectHandle(), DependencyObjectState::Reload);
    }

} // namespace pulsar