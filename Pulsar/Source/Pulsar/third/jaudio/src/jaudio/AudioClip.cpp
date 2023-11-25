#include "AudioClip.h"
#include "ALInclude.h"
#include "stb_vorbis.h"

namespace jaudio
{

    bool AudioClip::IsCreatedAudioResource() const
    {
        return m_createdResource;
    }

    void AudioClip::CreateAudioResource()
    {
        alGenBuffers(1, &m_buffer);
        alBufferData(m_buffer, AL_FORMAT_STEREO16, m_data.Data.data(), m_data.Data.size(), m_data.SampleRate);
        m_createdResource = true;
    }

    void AudioClip::DestroyAudioResource()
    {
        alDeleteBuffers(1, &m_buffer);
        m_createdResource = false;
        m_buffer = 0;
    }

    void AudioClip::ResetData(AudioClipData&& data)
    {
        m_data = std::move(data);
    }

    AudioClipData AudioClipHelper::Load(std::filesystem::path path)
    {
        AudioClipData clip;

        int err;
        auto vorb = stb_vorbis_open_filename("mu.ogg", &err, nullptr);
        stb_vorbis_info info = stb_vorbis_get_info(vorb);
        clip.SampleRate = info.sample_rate;
        clip.Channels = info.channels;
        int totalSamples = stb_vorbis_stream_length_in_samples(vorb) * info.channels;
        size_t len = sizeof(short) * totalSamples;
        clip.Data.resize(len);

        int numSamples = stb_vorbis_get_samples_short_interleaved(vorb, info.channels, (short*)clip.Data.data(), totalSamples);

        return clip;
    }
}