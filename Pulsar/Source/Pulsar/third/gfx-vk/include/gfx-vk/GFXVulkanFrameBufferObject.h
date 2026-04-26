#pragma once
#include "GFXVulkanTexture.h"
#include "VulkanInclude.h"
#include <gfx/GFXFrameBufferObject.h>

namespace gfx
{
	class GFXVulkanApplication;

	class GFXVulkanFrameBufferObject : public GFXFrameBufferObject
	{
		using base = GFXFrameBufferObject;
	public:
		/**
		* render target view
		*/
		GFXVulkanFrameBufferObject(GFXVulkanApplication* app,
			const std::vector<GFXTexture2DView_sp>& renderTargets);

		GFXVulkanFrameBufferObject(const GFXVulkanFrameBufferObject&) = delete;

		virtual ~GFXVulkanFrameBufferObject() override;
	public:
		virtual const array_list<GFXTexture2DView_sp>& GetRenderTargets() const override { return m_renderTargets; }

		VkExtent2D GetVkExtent() const { return { (uint32_t)m_width, (uint32_t)m_height }; }

		virtual int32_t GetWidth() const override { return m_width; }
		virtual int32_t GetHeight() const override { return m_height; }
		virtual const GFXRenderTargetDesc& GetRenderTargetDesc() const override { return m_renderTargetDesc; }

	protected:
		GFXRenderTargetDesc m_renderTargetDesc;

		std::vector<GFXTexture2DView_sp> m_renderTargets;

		GFXVulkanApplication* m_app;

		int32_t m_width;
		int32_t m_height;

		int32_t m_id;
	};
}