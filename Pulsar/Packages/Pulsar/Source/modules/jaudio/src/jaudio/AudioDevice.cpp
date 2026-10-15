#include "AudioDevice.h"
#include "ALInclude.h"
#include <cassert>

namespace jaudio
{
    class OpenALAudioDeviceContext : public AudioDeviceContext
    {
    public:
        ALCdevice* m_device = nullptr;
        ALCcontext* m_context = nullptr;
    };

#define M_CONTEXT static_cast<OpenALAudioDeviceContext*>(m_context)

    void AudioDevice::Initialize(const char* deviceName)
    {
        assert(!m_context);

        m_context = new OpenALAudioDeviceContext;

        M_CONTEXT->m_device = alcOpenDevice(deviceName);
        assert(M_CONTEXT->m_device);
        M_CONTEXT->m_context = alcCreateContext(M_CONTEXT->m_device, nullptr);
        assert(m_context);
        assert(alcMakeContextCurrent(M_CONTEXT->m_context));

        const ALCchar* name = nullptr;
        if (alcIsExtensionPresent(M_CONTEXT->m_device, "ALC_ENUMERATE_ALL_EXT"))
        {
            name = alcGetString(M_CONTEXT->m_device, ALC_ALL_DEVICES_SPECIFIER);
        }
        if (!name || alcGetError(M_CONTEXT->m_device) != AL_NO_ERROR)
        {
            name = alcGetString(M_CONTEXT->m_device, ALC_DEVICE_SPECIFIER);
        }

    }

    void AudioDevice::Terminate()
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(M_CONTEXT->m_context);
        alcCloseDevice(M_CONTEXT->m_device);

        delete m_context;
    }

}