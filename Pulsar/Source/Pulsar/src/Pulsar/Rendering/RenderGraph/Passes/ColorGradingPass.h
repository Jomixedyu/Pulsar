#pragma once
#include "PostProcessPass.h"
#include <Pulsar/Assets/ColorGradingSettings.h>

namespace pulsar
{
    class ColorGradingPass : public PostProcessPass
    {
    public:
        ColorGradingPass();

        void ReadSettings(const VolumeStack& stack) override;

    protected:
        void PrepareMaterial(CameraComponent* cam) override;
        bool IsEnabled() const override;
        const char* GetPassName() const override { return "PostProcess_LUT"; }

    private:
        void EnsureMaterial();

        const ColorGradingSettings* m_settings = nullptr;
    };
}
