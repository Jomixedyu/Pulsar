#pragma once

#include <Pulsar/ObjectBase.h>
#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Texture.h>
#include <Pulsar/IBindGPU.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <gfx/GFXShaderPass.h>
#include <Pulsar/Rendering/Types.h>

namespace pulsar
{
    class Texture2D;

    class ShaderPassConfig : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::ShaderPassConfig, Object);

    public:
        CORELIB_REFL_DECL_FIELD(VertShaderName);
        string VertShaderName;

        CORELIB_REFL_DECL_FIELD(PixelShaderName);
        string PixelShaderName;
        
        CORELIB_REFL_DECL_FIELD(CullMode);
        CullMode CullMode;

        CORELIB_REFL_DECL_FIELD(DepthTestEnable);
        bool DepthTestEnable;

        CORELIB_REFL_DECL_FIELD(DepthWriteEnable);
        bool DepthWriteEnable;

        CORELIB_REFL_DECL_FIELD(DepthCompareOp);
        CompareMode DepthCompareOp;

        CORELIB_REFL_DECL_FIELD(StencilTestEnable);
        bool StencilTestEnable;
    };
    CORELIB_DECL_SHORTSPTR(ShaderPassConfig);

    enum class ShaderParameterType
    {
        Float,
        Sampler2D,
        Vector3
    };

    class ShaderParameter
    {

    };

    class ShaderConfig
    {

        array_list<ShaderPassConfig_sp> configs;
    };

    struct ShaderPassSerializeData
    {
        ShaderPassConfig_sp Config;
        array_list<uint8_t> VertBytes;
        array_list<uint8_t> PixelBytes;
    };
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool isWrite, ShaderPassSerializeData& data);



    class Shader final: public AssetObject, public IBindGPU
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::Shader, AssetObject);
    public:
        virtual void OnSerialize(AssetSerializer* serializer) override;

        static sptr<Shader> StaticCreate(
            string_view name, 
            array_list<ShaderPassSerializeData>&& pass);

    public:
        virtual void BindGPU() override;
        virtual void UnBindGPU() override;
        virtual bool GetIsBindGPU() override;
    private:
        array_list<ShaderPassSerializeData> m_shaderSource;
    private:
        array_list<std::shared_ptr<gfx::GFXShaderPass>> m_shaderPass;
    };
    CORELIB_DECL_SHORTSPTR(Shader);



    class ShaderCompileException : public EngineException
    {
        
    public:
        virtual const char* name() const override { return "ShaderCompileException"; }

        ShaderCompileException(const string& name, const string& msg) : EngineException(msg), name_(name)
        {
            this->message_.insert(0, "filename: " + name + "");
        }

        string name_;
    };


}
