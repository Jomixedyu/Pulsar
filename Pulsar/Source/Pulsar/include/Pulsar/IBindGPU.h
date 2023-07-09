#pragma once

namespace pulsar
{
    class IBindGPU
    {
    public:
        virtual void BindGPU() = 0;
        virtual void UnBindGPU() = 0;
        virtual bool GetIsBindGPU() = 0;
    };
}