#include "Pulsar/Assets/VolumeProfile.h"

#include <CoreLib/Assembly.h>
#include <CoreLib.Serialization/JsonSerializer.h>

namespace pulsar
{

    VolumeProfile::VolumeProfile()
    {
        init_sptr_member(m_effects);
    }

    void VolumeProfile::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);

        if (s->IsWrite)
        {
            auto effectsArray = s->Object->New(ser::VarientType::Array);

            for (auto& effect : *m_effects)
            {
                auto effectObj = effectsArray->New(ser::VarientType::Object);
                effectObj->Add("Type", effect->GetType()->GetName());

                auto dataObj = effectObj->New(ser::VarientType::Object);
                auto js = ser::JsonSerializer::Serialize(effect.get(), {});
                dataObj->AssignParse(js);
                effectObj->Add("Data", dataObj);

                effectsArray->Push(effectObj);
            }

            s->Object->Add("Effects", effectsArray);
        }
        else
        {
            m_effects->clear();
            auto effectsArray = s->Object->At("Effects");
            if (!effectsArray)
                return;

            for (int i = 0; i < effectsArray->GetCount(); ++i)
            {
                auto effectObj = effectsArray->At(i);
                auto typeStr = effectObj->At("Type")->AsString();
                auto dataObj = effectObj->At("Data");

                auto effectType = AssemblyManager::GlobalFindType(typeStr);
                if (effectType)
                {
                    auto effect = jxcorlib::sptr_cast<VolumeSettings>(
                        ser::JsonSerializer::Deserialize(dataObj->ToString(), effectType));
                    if (effect)
                    {
                        m_effects->push_back(effect);
                    }
                }
            }
        }
    }

    void VolumeProfile::OnCollectAssetDependencies(array_list<guid_t>& deps)
    {
        for (auto& effect : *m_effects)
        {
            if (auto* ppMat = dynamic_cast<PostProcessMaterialSettings*>(effect.get()))
            {
                for (auto& mat : *ppMat->m_materials)
                {
                    if (mat)
                    {
                        auto guid = mat.GetGuid();
                        if (!guid.is_empty())
                            deps.push_back(guid);
                    }
                }
            }
        }
    }

} // namespace pulsar
