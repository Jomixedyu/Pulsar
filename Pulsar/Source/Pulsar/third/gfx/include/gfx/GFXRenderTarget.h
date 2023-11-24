#pragma once
#include "GFXTexture.h"
#include "GFXInclude.h"
#include <array>

namespace gfx
{
	enum class GFXRenderTargetType
	{
		Color,
		Depth,
		Stencil,
		DepthStencil
	};

	class GFXRenderTarget : public GFXTexture
	{
	public:
		virtual ~GFXRenderTarget() {}
		virtual GFXRenderTargetType GetRenderTargetType() const = 0;

		std::array<float, 4> ClearColor;
	};
	GFX_DECL_SPTR(GFXRenderTarget);
}