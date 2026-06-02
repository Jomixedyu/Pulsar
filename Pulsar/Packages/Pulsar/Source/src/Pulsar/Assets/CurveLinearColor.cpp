#include "Pulsar/Assets/CurveLinearColor.h"

#include "CoreLib.Serialization/JsonSerializer.h"

namespace pulsar
{

    CurveLinearColor::CurveLinearColor()
    {

    }

    static ser::VarientRef SaveCollection(ser::VarientRef dat, CurveKeyCollection& collection)
    {
        auto keysArr = dat->New(ser::VarientType::Array);
        for (auto& key : collection.Keys)
        {
            auto collectionObj = dat->New(ser::VarientType::Object);
            auto serialized = ser::JsonSerializer::Serialize(mkbox(key).get(), {});
            collectionObj->AssignParse(serialized);
            keysArr->Push(collectionObj);
        }
        return keysArr;
    }
    static void LoadCollection(ser::VarientRef keys, CurveKeyCollection& collection)
    {
        for (auto i = 0; i < keys->GetCount(); ++i)
        {
            auto curveObj = keys->At(i);
            auto key = ser::JsonSerializer::Deserialize<BoxingCurveKey>(curveObj->ToString())->get_unboxing_value();
            collection.AddKey(key);
        }
    }

    void CurveLinearColor::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (s->IsWrite)
        {
            auto curvesArray = s->Object->New(ser::VarientType::Object);

            curvesArray->Add("R", SaveCollection(curvesArray, m_R));
            curvesArray->Add("G", SaveCollection(curvesArray, m_G));
            curvesArray->Add("B", SaveCollection(curvesArray, m_B));
            curvesArray->Add("A", SaveCollection(curvesArray, m_A));

            s->Object->Add("Curves", curvesArray);
        }
        else
        {
            m_R = {};
            m_G = {};
            m_B = {};
            m_A = {};

            auto obj = s->Object->At("Curves");
            LoadCollection(obj->At("R"), m_R);
            LoadCollection(obj->At("G"), m_G);
            LoadCollection(obj->At("B"), m_B);
            LoadCollection(obj->At("A"), m_A);

        }
    }

    CurveKeyCollection CurveLinearColor::GetCurveData(size_t index) const
    {
        if (index == 0)
            return m_R;
        if (index == 1)
            return m_G;
        if (index == 2)
            return m_B;
        if (index == 3)
            return m_A;
        return {};
    }

    void CurveLinearColor::SetKeyR(int index, CurveKey r)
    {
        m_R.SetKey(index, r);
    }
    void CurveLinearColor::SetKeyG(int index, CurveKey g)
    {
        m_G.SetKey(index, g);
    }
    void CurveLinearColor::SetKeyB(int index, CurveKey b)
    {
        m_B.SetKey(index, b);
    }
    void CurveLinearColor::SetKeyA(int index, CurveKey a)
    {
        m_A.SetKey(index, a);
    }

    void CurveLinearColor::SetAllColorKey(int index, CurveKey all)
    {
        SetKeyR(index, all);
        SetKeyG(index, all);
        SetKeyB(index, all);
        SetKeyA(index, all);
    }

    CurveKey CurveLinearColor::GetKeyR(int index) const
    {
        return m_R.GetKey(index);
    }
    CurveKey CurveLinearColor::GetKeyG(int index) const
    {
        return m_G.GetKey(index);
    }
    CurveKey CurveLinearColor::GetKeyB(int index) const
    {
        return m_B.GetKey(index);
    }
    CurveKey CurveLinearColor::GetKeyA(int index) const
    {
        return m_A.GetKey(index);
    }

    std::array<CurveKey, 4> CurveLinearColor::GetColorsKey(int index) const
    {
        return std::array<CurveKey, 4>{
            GetKeyR(index),
            GetKeyG(index),
            GetKeyB(index),
            GetKeyA(index),
        };
    }
    void CurveLinearColor::SetColorsKey(int index, const std::array<CurveKey, 4>& colors)
    {
        SetKeyR(index, colors[0]);
        SetKeyG(index, colors[1]);
        SetKeyB(index, colors[2]);
        SetKeyA(index, colors[3]);
    }
    void CurveLinearColor::AddKey(CurveKey key)
    {
        m_R.AddKey(key);
        m_G.AddKey(key);
        m_B.AddKey(key);
        m_A.AddKey(key);
    }

    void CurveLinearColor::ClearKeys()
    {
        m_R.Keys.clear();
        m_G.Keys.clear();
        m_B.Keys.clear();
        m_A.Keys.clear();
    }
    void CurveLinearColor::AddKeyR(CurveKey key)
    {
        m_R.AddKey(key);
    }
    void CurveLinearColor::AddKeyG(CurveKey key)
    {
        m_G.AddKey(key);
    }
    void CurveLinearColor::AddKeyB(CurveKey key)
    {
        m_B.AddKey(key);
    }
    void CurveLinearColor::AddKeyA(CurveKey key)
    {
        m_A.AddKey(key);
    }

    Color4f CurveLinearColor::SampleColor(float t) const
    {
        Color4f color;
        color.r = m_R.Sample(t);
        color.g = m_G.Sample(t);
        color.b = m_B.Sample(t);
        color.a = m_A.Sample(t);

        return color;
    }

    void CurveLinearColor::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        RuntimeObjectManager::NotifyDependencySource(GetObjectHandle(), DependencyObjectState::Modified);
    }

} // namespace pulsar