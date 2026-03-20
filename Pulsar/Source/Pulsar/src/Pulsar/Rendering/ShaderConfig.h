#pragma once
#include <Pulsar/Assembly.h>
#include "Types.h"

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
                     ShaderPassRenderQueueType,
                     Opaque,
                     AlphaTest,
                     Transparency);
}
CORELIB_DECL_BOXING(pulsar::ShaderPassRenderQueueType, pulsar::BoxingShaderPassRenderQueueType);
namespace pulsar
{

    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
                     ShaderParameterType,
                     Int,
                     Float,
                     Float4,
                     Color,
                     Texture2D,
                     TextureCube
    )
}
CORELIB_DECL_BOXING(pulsar::ShaderParameterType, pulsar::BoxingShaderParameterType);

namespace pulsar
{
    struct Mask8
    {
        uint8_t Value{};

        Mask8() = default;
        Mask8(uint8_t value) : Value(value) {}

        static Mask8 parse(std::string_view str)
        {
            Mask8 result{};
            int base = 0; // auto-detect: 0x→16, 0→8, otherwise 10
            const char* begin = str.data();
            if (str.starts_with("0b") || str.starts_with("0B"))
            {
                begin += 2;
                base = 2;
            }
            result.Value = static_cast<uint8_t>(std::strtoul(begin, nullptr, base));
            return result;
        }

        std::string to_string() const
        {
            return std::to_string(Value);
        }
    };

    class BoxingMask8 : public BoxingObject, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxingMask8, Object);
        CORELIB_IMPL_INTERFACES(IStringify);
    public:
        using unboxing_type = Mask8;
        auto get_unboxing_value() { return m_mask; }

        BoxingMask8() : CORELIB_INIT_INTERFACE(IStringify) {}
        BoxingMask8(Mask8 value) : m_mask(value), CORELIB_INIT_INTERFACE(IStringify) {}

        void IStringify_Parse(const string& value) override
        {
            m_mask = Mask8::parse(value);
        }
        string IStringify_Stringify() override
        {
            return m_mask.to_string();
        }

        string ToString() const override
        {
            return m_mask.to_string();
        }

        Mask8 m_mask{};
    };
}
CORELIB_DECL_BOXING(pulsar::Mask8, pulsar::BoxingMask8);

namespace pulsar
{

}

namespace pulsar
{
    class ShaderConfigProperty : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ShaderConfigProperty, Object);
    public:
        CORELIB_REFL_DECL_FIELD(Name);
        string Name;

        CORELIB_REFL_DECL_FIELD(Label);
        string Label;

        CORELIB_REFL_DECL_FIELD(Type);
        ShaderParameterType Type{};

        CORELIB_REFL_DECL_FIELD(DefaultValue);
        string DefaultValue;

    };

    class ShaderConfigEntry : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ShaderConfigEntry, Object);
    public:
        CORELIB_REFL_DECL_FIELD(Vertex);
        string Vertex;
        
        CORELIB_REFL_DECL_FIELD(Fragment);
        string Fragment;
        
        CORELIB_REFL_DECL_FIELD(TessControl);
        string TessControl;
        
        CORELIB_REFL_DECL_FIELD(TessEval);
        string TessEval;
    };


    class ShaderConfigGraphicsPipeline : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ShaderConfigGraphicsPipeline, Object);
    public:
        CORELIB_REFL_DECL_FIELD(CullMode);
        CullMode CullMode = CullMode::Back;

        CORELIB_REFL_DECL_FIELD(ZTestOp);
        CompareMode ZTestOp = gfx::GFXCompareMode::Less;

        CORELIB_REFL_DECL_FIELD(ZWriteEnabled);
        bool ZWriteEnabled = true;

        CORELIB_REFL_DECL_FIELD(ZClipEnabled);
        bool ZClipEnabled = false;


        CORELIB_REFL_DECL_FIELD(Stencil_Enabled);
        bool Stencil_Enabled = false;

        CORELIB_REFL_DECL_FIELD(Stencil_Ref);
        Mask8 Stencil_Ref = 0;

        CORELIB_REFL_DECL_FIELD(Stencil_ReadMask);
        Mask8 Stencil_ReadMask = 0xFF;

        CORELIB_REFL_DECL_FIELD(Stencil_WriteMask);
        Mask8 Stencil_WriteMask = 0xFF;

        CORELIB_REFL_DECL_FIELD(Stencil_CompareOp);
        CompareMode Stencil_CompareOp = CompareMode::Always;

        CORELIB_REFL_DECL_FIELD(Stencil_PassOp);
        StencilOp Stencil_PassOp = StencilOp::Keep;

        CORELIB_REFL_DECL_FIELD(Stencil_FailOp);
        StencilOp Stencil_FailOp = StencilOp::Keep;

        CORELIB_REFL_DECL_FIELD(Stencil_ZFailOp);
        StencilOp Stencil_ZFailOp = StencilOp::Keep;


        CORELIB_REFL_DECL_FIELD(Blend_Enabled);
        bool Blend_Enabled = false;

        CORELIB_REFL_DECL_FIELD(Blend_AlphaEnabled);
        bool Blend_AlphaEnabled = false;

        CORELIB_REFL_DECL_FIELD(Blend_Src);
        BlendFactor Blend_Src = BlendFactor::One;

        CORELIB_REFL_DECL_FIELD(Blend_Dst);
        BlendFactor Blend_Dst = BlendFactor::Zero;

        CORELIB_REFL_DECL_FIELD(Blend_SrcAlpha);
        BlendFactor Blend_SrcAlpha = BlendFactor::Zero;

        CORELIB_REFL_DECL_FIELD(Blend_DstAlpha);
        BlendFactor Blend_DstAlpha = BlendFactor::One;

    };

    class ShaderConfigPass : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ShaderConfigPass, Object);
    public:
        CORELIB_REFL_DECL_FIELD(Name);
        string Name;

        CORELIB_REFL_DECL_FIELD(Queue);
        ShaderPassRenderQueueType Queue;

        CORELIB_REFL_DECL_FIELD(Entry);
        List_sp<SPtr<ShaderConfigEntry>> Entry;

        CORELIB_REFL_DECL_FIELD(Features);
        List_sp<string> Features;

        CORELIB_REFL_DECL_FIELD(GraphicsPipeline);
        SPtr<ShaderConfigGraphicsPipeline> GraphicsPipeline;
    };

    class ShaderConfig : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ShaderConfig, Object);

    public:
        CORELIB_REFL_DECL_FIELD(PreDefines);
        List_sp<string> PreDefines;
        
        CORELIB_REFL_DECL_FIELD(Tags);
        List_sp<string> Tags;
        
        CORELIB_REFL_DECL_FIELD(Interfaces);
        List_sp<string> Interfaces;
        
        CORELIB_REFL_DECL_FIELD(Passes);
        List_sp<SPtr<ShaderConfigPass>> Passes;
        
        CORELIB_REFL_DECL_FIELD(Properties);
        List_sp<SPtr<ShaderConfigProperty>> Properties;
    };
    CORELIB_DECL_SHORTSPTR(ShaderConfig);


}
