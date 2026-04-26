#include "rdg/RDGBuilder.h"

namespace rdg
{

    RDGTexture* RDGBuilder::CreateTexture(std::string_view name, const RDGTextureDesc& desc)
    {
        auto tex = new RDGTexture(name, desc, m_textures.size());

        m_textures.push_back(tex);
        return tex;
    }

    RDGBuffer* RDGBuilder::CreateBuffer(std::string_view name, const RDGBufferDesc& desc)
    {
        auto buffer = new RDGBuffer();

        m_buffers.push_back(buffer);
        return buffer;
    }

    void RDGBuilder::Compile()
    {
        // cull

    }

    void RDGBuilder::Execute()
    {
        // create resource
        // execute command

    }
} // namespace rdg