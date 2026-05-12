#pragma once
#include <Pulsar/Assembly.h>
#include <Pulsar/AssetObject.h>
#include <CoreLib/Reflection.h>

namespace pulsar
{
    class ObjectPropertyOverride : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ObjectPropertyOverride, Object);
    public:
        ObjectPropertyOverride()
        {
            init_sptr_member(Paths);
        }

        CORELIB_REFL_DECL_FIELD(Paths);
        List_sp<string> Paths;

        bool IsEmpty() const { return !Paths || Paths->empty(); }
        void AddField(const string& name);

        void ApplyTo(Object* source, Object* override, Object* target) const;
    };
}
