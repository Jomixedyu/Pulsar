#include "Enum.h"

namespace jxcorlib
{
    void Enum::InitDefinitions(const char* datas, DataMap** defs)
    {
        if (*defs) return;
        *defs = new DataMap;
        auto enums = StringUtil::Split(datas, u8char(","));
        for (string& enum_item : enums) {
            auto kv = StringUtil::Split(enum_item, u8char("="));

            string& name_ = StringUtil::TrimSelf(kv[0]);
            uint32_t value_ = 0;

            if (kv.size() == 2)
            {
                string& enum_ = StringUtil::TrimSelf(kv[1]);
                value_ = (uint32_t)std::stoul(enum_);
            }
            else if (kv.size() == 1)
            {
                if ((*defs)->size() > 0) // last + 1
                {
                    value_ = (*defs)->at((*defs)->size() - 1).second + 1;
                }
                else // == 0
                {
                    value_ = 0;
                }
            }
            (*defs)->push_back({ name_, value_ });
        }
    }
    bool Enum::StaticTryParse(Type* type, string_view name, uint32_t* out_value)
    {
        for (const auto& [_name, _value] : *type->GetEnumDefinitions())
        {
            if (_name == name)
            {
                *out_value = _value;
                return true;
            }
        }
        return false;
    }
}