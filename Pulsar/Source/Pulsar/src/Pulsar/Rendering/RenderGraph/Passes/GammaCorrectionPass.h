#pragma once
#include "PostProcessPass.h"
#include <Pulsar/Assets/GammaCorrectionSettings.h>

namespace pulsar
{
    class GammaCorrectionPass : public PostProcessPass
    {
    public:
        GammaCorrectionPass();

        void ReadSettings(const VolumeStack& stack) override;

    protected:
        void PrepareMaterial(CameraComponent* cam) override;
        bool IsEnabled() const override { return m_settings != nullptr && m_settings->m_enabled; }
        const char* GetPassName() const override { return "PostProcess_Gamma"; }

    private:
        void EnsureMaterial();

        const GammaCorrectionSettings* m_settings = nullptr;
    };
}
