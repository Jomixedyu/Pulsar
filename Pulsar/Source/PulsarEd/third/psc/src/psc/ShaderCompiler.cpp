
#include "psc/ShaderCompiler.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include "cxxopts.hpp"
#include "stlext.hpp"
#include "GlslangCompilerImpl.h"
#include "DxcCompilerImpl.h"

namespace psc
{
    std::shared_ptr<ShaderCompiler> CreateShaderCompiler(ApiPlatformType platform)
    {
        switch (platform)
        {
        case ApiPlatformType::Vulkan:
        case ApiPlatformType::OpenGL:
        case ApiPlatformType::OpenGLES:
            return std::make_shared<GlslangCompilerImpl>();
            break;
        case ApiPlatformType::Direct3D:
            //return std::make_shared<DxcCompilerImpl>();
        default:
            throw 0;
        }
        return {};
    }

}
