#pragma once

namespace pulsar
{
    class IGPUResource
    {
    public:
        // Ensures the GPU-side representation exists and matches the current
        // asset data: created on first call, re-synced when the data changed.
        // Cheap when nothing changed -- call when binding the asset to a user
        // and after any data edit/reload.
        virtual bool EnsureGPUResource() = 0;
        virtual void DestroyGPUResource() = 0;
        virtual bool IsCreatedGPUResource() const = 0;
    };
}