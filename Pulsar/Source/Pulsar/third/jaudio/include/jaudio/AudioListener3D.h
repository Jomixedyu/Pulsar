#pragma once
#include "Point3D.h"
#include "IAudioResource.h"

namespace jaudio
{
    class JAUDIO_API AudioListener3D : public Point3D, public IAudioResource
    {
    public:
        virtual bool IsCreatedAudioResource() const override;
        virtual void CreateAudioResource() override;
        virtual void DestroyAudioResource() override;

    protected:
    };
}