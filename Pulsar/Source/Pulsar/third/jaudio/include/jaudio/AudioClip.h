#pragma once
#include "Define.h"
#include "IAudioResource.h"
#include <vector>
#include <string_view>
#include <filesystem>
#include <memory>

namespace jaudio
{
    struct AudioWaveBuffer
    {
        std::vector<uint8_t> Data;
        int SampleRate;
        int Channels;
    };

    class JAUDIO_API AudioClip : public IAudioResource
    {
    public:
        virtual void ResetData(AudioWaveBuffer&& data);

        virtual bool IsCreatedAudioResource() const override;
        virtual void CreateAudioResource() override;
        virtual void DestroyAudioResource() override;

        uint32_t GetBuffer() const { return m_buffer; }
    private:
        int m_channelCount;
        size_t m_sampleCount;
        bool m_createdResource;
        uint32_t m_buffer;
        AudioWaveBuffer m_data;
    };



    class JAUDIO_API AudioClipHelper
    {
    public:
        static AudioWaveBuffer Load(std::filesystem::path path);
    };
};