#pragma once
#include "RDGInclude.h"
#include "RDGTexture.h"
#include "gfx/GFXCommandBuffer.h"
#include "gfx/GFXGraphicsPipelineManager.h"

namespace rdg
{
    struct CommandBufferContext
    {
        GFXCommandBuffer& Cmd;
        GFXGraphicsPipelineManager& PSOMgr;
    };


    struct ShaderParameters
    {
        virtual ~ShaderParameters() = default;
        virtual int GetFields() { return {}; }
    };

    class RDGPass
    {
    public:
        RDGPass(std::string_view name);

        std::string_view GetName() const { return m_name; }

        virtual void Execute(CommandBufferContext& cmd) {}
        virtual ~RDGPass() {}

        bool Culled;

    protected:
        std::string m_name;
    };

    template<typename LAMBDA>
    class RDGLambdaPass : public RDGPass
    {
    public:
        RDGLambdaPass(std::string_view name, LAMBDA&& lambda) : RDGPass(name), m_lambda(std::move(lambda))
        {
        }

        void Execute(CommandBufferContext& cmd) override
        {
            m_lambda(cmd);
        }

        LAMBDA m_lambda;
    };
}