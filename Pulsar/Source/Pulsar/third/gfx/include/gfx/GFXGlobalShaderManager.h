#pragma once
#include "GFXGpuProgram.h"

namespace gfx
{
    class GFXGlobalShaderManager
    {
    public:
        void RegisterGpuProgram(string_view name, const std::shared_ptr<GFXGpuProgram>& gpuProgram);

        void Clear() { m_programs.clear(); }

        array_list<std::shared_ptr<GFXGpuProgram>> m_programs;
    };
}