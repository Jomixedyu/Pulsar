#pragma once
#include "GFXTexture.h"

namespace gfx
{
	enum class GFXRenderTargetType
	{
		Color,
		Depth
	};

	class GFXRenderTarget : public GFXTexture
	{
	public:
		virtual ~GFXRenderTarget() {}
	};
}