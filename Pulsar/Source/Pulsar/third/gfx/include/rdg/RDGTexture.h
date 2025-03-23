#pragma once
#include "RDGResource.h"
#include <gfx/GFXTexture.h>

namespace rdg
{

    struct RDGTextureDesc : public GFXTextureCreateDesc
    {

    };

    class RDGTexture : public RDGResource
    {
    public:
        using base = RDGResource;
        using RDGHandle = int;

        RDGTexture(std::string_view name, const RDGTextureDesc& desc, int handle)
            : m_name(name), m_desc(desc), m_handle(handle)
        {

        }


        std::string m_name;
        RDGTextureDesc m_desc;
        RDGHandle m_handle;
    };
}