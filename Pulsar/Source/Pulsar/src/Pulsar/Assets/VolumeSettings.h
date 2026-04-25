#pragma once
#include <Pulsar/Assembly.h>
#include <CoreLib/Object.h>

namespace pulsar
{
    class VolumeSettings : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::VolumeSettings, Object);
    public:
        // Base class for all volume effect settings.
        // Derived classes should add their own parameters with CORELIB_REFL_DECL_FIELD.
    };

    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar, TonemappingMode, None, ACES, Reinhard, GT);

} // namespace pulsar

// Boxing declaration must appear before any class that uses the enum in CORELIB_REFL_DECL_FIELD.
CORELIB_DECL_BOXING(pulsar::TonemappingMode, pulsar::BoxingTonemappingMode);
