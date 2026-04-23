
#include "ShaderConfig.h"
#include <Pulsar/Meta/PropertyStyleAttributes.h>

#include <CoreLib/Attribute.h>
#include <CoreLib/Assembly.h>

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
}
