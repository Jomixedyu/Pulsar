#pragma once
#include "Define.h"

namespace jaudio
{

    class JAUDIO_API AudioDeviceContext
    {
    public:
        virtual ~AudioDeviceContext() {}
    };

    class JAUDIO_API AudioDevice
    {
    public:
        void Initialize(const char* deviceName);
        void Terminate();

    protected:
        AudioDeviceContext* m_context = nullptr;
    };


}