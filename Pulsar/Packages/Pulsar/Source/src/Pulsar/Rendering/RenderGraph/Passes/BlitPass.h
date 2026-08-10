#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/Rendering/MaterialProxy.h>
#include <gfx/GFXBuffer.h>
#include <memory>
#include <string>

namespace pulsar
{
    class BlitPass
    {
    public:
        struct Desc
        {
            std::string Name = "Blit";
            RGTextureHandle Source;
            RGTextureHandle Destination;
            std::shared_ptr<MaterialProxy> Material;
            std::string ShaderPass = "Blit";
            std::string Interface = "RENDERER_IMAGEPROCESS";
            std::string SourceBindingName = "_MainTex";
            gfx::GFXBuffer* CameraBuffer = nullptr;
            gfx::GFXBuffer* WorldBuffer = nullptr;
            RGAttachmentDesc Attachment{};
        };

        static RGTextureHandle AddToGraph(RenderGraph& graph, Desc desc);
    };
}