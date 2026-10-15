#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
// vulkan.h drags in <windows.h> for the win32 platform bindings; keep its
// min/max macros from colliding with std::min/std::max.
#define NOMINMAX
#include <vulkan/vulkan.h>
#include "gfx/GFXInclude.h"
#include <cassert>