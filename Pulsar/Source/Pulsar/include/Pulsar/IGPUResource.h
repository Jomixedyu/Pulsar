#pragma once

namespace pulsar
{
    class IGPUResource
    {
    public:
        virtual bool CreateGPUResource() = 0;
        virtual void DestroyGPUResource() = 0;
        virtual bool IsCreatedGPUResource() const = 0;
    };
}