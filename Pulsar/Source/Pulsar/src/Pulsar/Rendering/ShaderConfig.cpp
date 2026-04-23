
#include "ShaderConfig.h"
#include <Pulsar/Meta/PropertyStyleAttributes.h>

#include <CoreLib/Attribute.h>
#include <CoreLib/Assembly.h>

namespace pulsar
{
    std::vector<SPtr<jxcorlib::Attribute>> ShaderConfigProperty::GetStyleAttributes() const
    {
        std::vector<SPtr<jxcorlib::Attribute>> result;
        if (Style.empty())
            return result;

        static const auto editStyleTypes = []() {
            return jxcorlib::AssemblyManager::GlobalSearchType(cltypeof<EditStyleAttribute>());
        }();

        for (auto* type : editStyleTypes)
        {
            auto temp = jxcorlib::sptr_static_cast<EditStyleAttribute>(type->CreateSharedInstance({}));
            if (temp && temp->TryParse(Style))
            {
                auto attrs = temp->Parse(Style);
                result.insert(result.end(), attrs.begin(), attrs.end());
            }
        }
        return result;
    }
}
