#pragma once
#include <Pulsar/Assembly.h>
#include <CoreLib/Guid.h>
#include <CoreLib/Object.h>

namespace pulsar
{
    class VolumeSettings : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::VolumeSettings, Object);
    public:
        // Whether this effect is enabled in the volume.
        virtual bool IsEnabled() const { return true; }

        // Returns false for effect types that do not participate in numeric blending
        // (e.g. material lists are handled separately).
        virtual bool SupportsBlending() const { return true; }

        // Blend this setting's parameters into the accumulator.
        // The accumulator is guaranteed to be of the same derived type.
        virtual void Blend(float weight, VolumeSettings* accumulator) = 0;

        // Collect asset GUIDs referenced by this settings object.
        virtual void CollectAssetDependencies(array_list<guid_t>& deps) {}
    };

    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar, TonemappingMode, None, ACES, Reinhard, GT);

} // namespace pulsar

// Boxing declaration must appear before any class that uses the enum in CORELIB_REFL_DECL_FIELD.
CORELIB_DECL_BOXING(pulsar::TonemappingMode, pulsar::BoxingTonemappingMode);
