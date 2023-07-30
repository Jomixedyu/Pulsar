#pragma once
#include <gfx/GFXRenderTarget.h>
#include "VulkanInclude.h"
#include "GFXVulkanTexture2D.h"
#include <cassert>
#include "GFXVulkanRenderPass.h"
#include <gfx/GFXFrameBufferObject.h>
#include "GFXVulkanRenderTarget.h"

namespace gfx
{
	class GFXVulkanApplication;

	class GFXVulkanFrameBufferObject : public GFXFrameBufferObject
	{
		using base = GFXRenderTarget;
	public:
		/**
		* render target view
		*/
		GFXVulkanFrameBufferObject(GFXVulkanApplication* app, 
			const std::vector<GFXVulkanRenderTarget*>& renderTargets, 
			const std::shared_ptr<GFXVulkanRenderPass>& renderPass);

		GFXVulkanFrameBufferObject(const GFXVulkanFrameBufferObject&) = delete;

		virtual ~GFXVulkanFrameBufferObject() override;
	public:
		//VkImage GetVkColorImage() const { return m_tex2d ? m_tex2d->GetVkImage() : VK_NULL_HANDLE; }
		//VkImageView GetVkColorImageView() const { return m_tex2d ? m_tex2d->GetVkImageView() : VK_NULL_HANDLE; }
		//VkImageLayout GetVkColorImageLayout() const { assert(m_tex2d); return m_tex2d->GetVkImageLayout(); }
		//VkImageLayout GetVkDepthImageLayout() const { return m_depthTex->GetVkImageLayout(); }
		//VkImage GetVkDepthImage() const { return m_depthTex ? m_depthTex->GetVkImage() : VK_NULL_HANDLE; }
		//VkImageView GetVkDepthImageView() const { return m_depthTex ? m_depthTex->GetVkImageView() : VK_NULL_HANDLE; }

		VkFramebuffer GetVkFrameBuffer() const { return m_frameBuffer; }
		const std::vector<GFXVulkanRenderTarget*>& GetRenderTargets() const { return m_renderTargets; }
		VkExtent2D GetVkExtent() const { return { (uint32_t)m_width, (uint32_t)m_height }; }

		virtual int32_t GetWidth() const override { return m_width; }
		virtual int32_t GetHeight() const override { return m_height; }
		virtual std::shared_ptr<GFXRenderPassLayout> GetRenderPassLayout() const override { return m_renderPass; }

		VkRenderPass GetVkRenderPass() const { return m_renderPass->GetVkRenderPass(); }

	protected:
		void InitRenderPass();
		void TermRenderPass();
	protected:
		//std::shared_ptr<GFXVulkanRenderPass> m_renderPass;

		std::shared_ptr<GFXVulkanRenderPass> m_renderPass;
		VkFramebuffer m_frameBuffer = VK_NULL_HANDLE;

		std::vector<GFXVulkanRenderTarget*> m_renderTargets;

		GFXVulkanApplication* m_app;

		int32_t m_width;
		int32_t m_height;
	};
}