#pragma once
#include "Define.h"
#include <cstdint>

namespace jaudio
{
    class JAUDIO_API IAudioResource
    {
    public:
        virtual bool IsCreatedAudioResource() const = 0;
        virtual void CreateAudioResource() = 0;
        virtual void DestroyAudioResource() = 0;
    };
}