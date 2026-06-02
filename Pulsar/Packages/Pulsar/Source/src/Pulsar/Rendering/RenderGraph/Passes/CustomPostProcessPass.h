#pragma once
#include "PostProcessPass.h"
#include <string>

namespace pulsar
{
    class CustomPostProcessPass : public PostProcessPass
    {
    public:
        CustomPostProcessPass();

        void SetPassName(std::string name) { m_passName = std::move(name); }
        void ReadSettings(const VolumeStack& stack) override {}

    protected:
        const char* GetPassName() const override { return m_passName.c_str(); }
    private:
        std::string m_passName;
    };
}
