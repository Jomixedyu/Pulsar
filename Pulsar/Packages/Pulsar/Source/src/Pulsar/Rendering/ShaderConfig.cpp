
#include "ShaderConfig.h"
#include <Pulsar/Meta/PropertyStyleAttributes.h>

#include <CoreLib/Attribute.h>
#include <CoreLib/Assembly.h>
#include <CoreLib.Serialization/JsonSerializer.h>

namespace pulsar
{
    SPtr<jxcorlib::Attribute> ShaderConfigProperty::GetStyleAttribute() const
    {
        if (Style.empty())
            return nullptr;

        static const auto editStyleTypes = []() {
            return jxcorlib::AssemblyManager::GlobalSearchType(cltypeof<EditStyleAttribute>());
        }();

        for (auto* type : editStyleTypes)
        {
            auto temp = jxcorlib::sptr_static_cast<EditStyleAttribute>(type->CreateSharedInstance({}));
            if (temp && temp->TryParse(Style))
            {
                return temp->Parse(Style);
            }
        }
        return nullptr;
    }

    SPtr<ShaderConfigPass> ShaderConfig::FindPass(const string& name) const
    {
        if (!Passes)
            return nullptr;
        for (const auto& pass : *Passes)
        {
            if (pass->Name == name)
                return pass;
        }
        return nullptr;
    }

    SPtr<ShaderConfig> ShaderConfig::Clone() const
    {
        auto json = ser::JsonSerializer::Serialize(const_cast<ShaderConfig*>(this), {});
        return ser::JsonSerializer::Deserialize<ShaderConfig>(json);
    }
}
