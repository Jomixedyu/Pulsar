#pragma once
#include "Point3D.h"
#include "IAudioResource.h"
#include "AudioClip.h"

namespace jaudio
{
    class JAUDIO_API AudioSource3D : public Point3D, public IAudioResource
    {
    public:
        virtual bool IsCreatedAudioResource() const override;
        virtual void CreateAudioResource() override;
        virtual void DestroyAudioResource() override;

        void Play();
        void Pause();
        void Stop();

        std::shared_ptr<AudioClip> GetAudioClip() const { return m_clip; }
        void SetAudioClip(const std::shared_ptr<AudioClip>& clip);

        float GetSpeed() const { return m_speed; }
        void SetSpeed(float value);

        float GetVolume() const { return m_volume; }
        void SetVolume(float value);

        bool IsPlaying() const { return m_isPlaying; }
    protected:
        virtual void OnPositionChanged() override;
    protected:
        uint32_t m_source = 0;
        std::shared_ptr<AudioClip> m_clip;
        float m_speed = 1;
        float m_volume = 1;
        bool m_isPlaying = false;
    };

}