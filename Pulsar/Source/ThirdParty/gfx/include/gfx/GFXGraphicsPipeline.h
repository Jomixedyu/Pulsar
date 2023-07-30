#pragma once

namespace gfx
{
    class GFXGraphicsPipeline
    {
    public:
        GFXGraphicsPipeline() {}
        GFXGraphicsPipeline(const GFXGraphicsPipeline&) = delete;
        GFXGraphicsPipeline(GFXGraphicsPipeline&&) = delete;
        virtual ~GFXGraphicsPipeline() {}
    public:

    };
}