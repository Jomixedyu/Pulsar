#pragma once
#include "PostProcessPass.h"
#include <Pulsar/Assets/TonemappingSettings.h>

namespace pulsar
{
    class TonemapPass : public PostProcessPass
    {
    public:
        TonemapPass();

        void ReadSettings(const VolumeStack& stack) override;

    protected:
        void PrepareMaterial(CameraComponent* cam) override;
        bool IsEnabled() const override { return m_settings != nullptr && m_settings->m_enabled; }
        const char* GetPassName() const override { return "PostProcess_Tonemap"; }

    private:
        void EnsureMaterial();

        const TonemappingSettings* m_settings = nullptr;
    };
}
