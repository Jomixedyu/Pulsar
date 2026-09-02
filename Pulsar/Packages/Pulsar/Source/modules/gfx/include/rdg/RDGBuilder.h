#pragma once
#include "RDGBuffer.h"
#include "RDGInclude.h"
#include "RDGPass.h"
#include "RDGTexture.h"
#include "gfx/GFXGraphicsPipelineManager.h"

namespace rdg
{
    using RDGTextureHandle = int;

    enum class RDGPassFlags
    {
        None,
        Raster = 1 << 0,
        Compute = 1 << 1,
        Copy = 1 << 2,
        NeverCull = 1 << 3,
    };

    template <typename T>
    concept CommandBufferCollectorConcept = requires { std::declval<T>()(std::declval<CommandBufferContext&>()); };


    class RDGBuilder final
    {
    public:
        RDGBuilder(const RDGBuilder&) = delete;
        RDGBuilder(RDGBuilder&&) = delete;
        RDGBuilder& operator=(const RDGBuilder&) = delete;
        RDGBuilder& operator=(RDGBuilder&&) = delete;

        RDGTexture* CreateTexture(std::string_view name, const RDGTextureDesc& desc);

        RDGBuffer* CreateBuffer(std::string_view name, const RDGBufferDesc& desc);

        template <CommandBufferCollectorConcept LAMBDA>
        RDGPass* AddPass(std::string_view name, RDGPassFlags flags, LAMBDA&& lambdaExecute)
        {
            auto pass = new RDGLambdaPass(name, std::forward<LAMBDA&&>(lambdaExecute));

            m_passes.push_back(pass);
            return pass;
        }

        void Compile();
        void Execute();

        std::vector<RDGPass*> m_passes;
        std::vector<RDGTexture*> m_textures;
        std::vector<RDGBuffer*> m_buffers;
    };
} // namespace rdg