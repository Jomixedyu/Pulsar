#pragma once
#include <gfx/GFXShader.h>
#include "VulkanInclude.h"

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanShader : public GFXShader
    {

        VkRenderPass pass;
    };
}