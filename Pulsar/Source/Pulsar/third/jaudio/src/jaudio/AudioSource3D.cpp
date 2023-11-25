#include "AudioSource3D.h"
#include "ALInclude.h"

namespace jaudio
{


    bool AudioSource3D::IsCreatedAudioResource() const
    {
        return false;
    }

    void AudioSource3D::CreateAudioResource()
    {
        alGenSources(1, &m_source);
    }

    void AudioSource3D::DestroyAudioResource()
    {
        alDeleteSources(1, &m_source);
    }

    void AudioSource3D::Play()
    {
        alSourcePlay(m_source);
        alListener3f(AL_POSITION, 0, 0, 0);
    }

    void AudioSource3D::Pause()
    {
        alSourcePause(m_source);
    }

    void AudioSource3D::Stop()
    {
        alSourceStop(m_source);
    }

    void AudioSource3D::OnPositionChanged()
    {
        alSource3f(m_source, AL_POSITION, m_x, m_y, m_z);
    }

    void AudioSource3D::SetAudioClip(const std::shared_ptr<AudioClip>& clip)
    {
        m_clip = clip;
        alSourcei(m_source, AL_BUFFER, m_clip->GetBuffer());
    }
    void AudioSource3D::SetSpeed(float value)
    {
        alSourcef(m_source, AL_SPEED_OF_SOUND, value);
        m_speed = value;
    }

    void AudioSource3D::SetVolume(float value)
    {
        alSourcef(m_source, AL_GAIN, value);
        m_volume = value;
    }

}