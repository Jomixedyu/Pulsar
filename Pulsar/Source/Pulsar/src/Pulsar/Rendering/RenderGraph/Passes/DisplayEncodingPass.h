#pragma once
#include "PostProcessPass.h"
#include <Pulsar/Assets/DisplayEncodingSettings.h>

namespace pulsar
{
    class DisplayEncodingPass : public PostProcessPass
    {
    public:
        DisplayEncodingPass();

        void ReadSettings(const VolumeStack& stack) override;

    protected:
        void PrepareMaterial(CameraComponent* cam) override;
        bool IsEnabled() const override { return m_settings != nullptr && m_settings->m_enabled; }
        const char* GetPassName() const override { return "PostProcess_DisplayEncoding"; }

    private:
        void EnsureMaterial();

        const DisplayEncodingSettings* m_settings = nullptr;
    };
}
