#pragma once
#include "Assembly.h"
#include "Assets/Material.h"
#include "Pulsar/Assets/RenderTexture.h"

namespace pulsar
{
    struct PostProcessingContext
    {
        RCPtr<RenderTexture> GetTempTarget(int width, int height);
        void ReleaseTempTarget(const RCPtr<RenderTexture>& tex);
        void Blit(RCPtr<RenderTexture> src, RCPtr<RenderTexture>& dest, RCPtr<Material> mat);

        RCPtr<RenderTexture> Source;
        RCPtr<RenderTexture> Target;
    };

    class PostProcessing : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::PostProcessing, Object);
    public:

        virtual void Render(PostProcessingContext* ctx) = 0;

        bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool value);
    protected:
        CORELIB_REFL_DECL_FIELD(m_enabled)
        bool m_enabled;
    };

    class BloomPostProcessing : public PostProcessing
    {
        void Render(PostProcessingContext* ctx) override
        {
            ctx->Blit(ctx->Source, ctx->Target, nullptr);
        }
    };
} // namespace pulsar
