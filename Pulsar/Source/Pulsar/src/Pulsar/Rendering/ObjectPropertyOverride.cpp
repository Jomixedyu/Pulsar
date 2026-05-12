#include "Rendering/ObjectPropertyOverride.h"

namespace pulsar
{
    void ObjectPropertyOverride::AddField(const string& name)
    {
        if (!Paths) return;
        for (const auto& path : *Paths)
        {
            if (path == name) return;
        }
        Paths->push_back(name);
    }

    void ObjectPropertyOverride::ApplyTo(Object* source, Object* override_, Object* target) const
    {
        if (!Paths)
            return;

        auto targetType = target->GetType();

        // 1. 全量复制 source -> target（source 为空则跳过）
        if (source)
        {
            auto sourceType = source->GetType();
            for (auto fieldInfo : sourceType->GetFieldInfos())
            {
                if (auto dstField = targetType->GetFieldInfo(fieldInfo->GetName()))
                {
                    dstField->SetValue(target, fieldInfo->GetValue(source));
                }
            }
        }

        // 2. 用 override 中被标记的字段覆盖 target
        auto overrideType = override_->GetType();
        for (const auto& path : *Paths)
        {
            auto fieldName = path;
            if (auto srcField = overrideType->GetFieldInfo(fieldName))
            {
                if (auto dstField = targetType->GetFieldInfo(fieldName))
                {
                    dstField->SetValue(target, srcField->GetValue(override_));
                }
            }
        }
    }
}
