#pragma once

#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Texture.h>
#include <Pulsar/IGPUResource.h>
#include <Pulsar/ObjectBase.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Rendering/Types.h>
#include <gfx/GFXApi.h>
#include <gfx/GFXShaderPass.h>
#include <mutex>

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
        ShaderPassRenderingType,
        OpaqueForward,
        OpaqueDeferred,
        Transparency,
        PostProcessing,
        );

    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
        ShaderParameterType,
        IntScalar,
        Scalar,
        Vector,
        Texture2D,
    )
}

CORELIB_DECL_BOXING(pulsar::ShaderPassRenderingType, pulsar::BoxingShaderPassRenderingType);
CORELIB_DECL_BOXING(pulsar::ShaderParameterType, pulsar::BoxingShaderParameterType);

namespace pulsar
{
    class Texture2D;

    class ShaderPassConfigProperty : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ShaderPassConfigProperty, Object);
    public:
        CORELIB_REFL_DECL_FIELD(Name);
        string Name;

        CORELIB_REFL_DECL_FIELD(Type);
        ShaderParameterType Type{};

        CORELIB_REFL_DECL_FIELD(Value);
        string Value;
    };
    CORELIB_DECL_SHORTSPTR(ShaderPassConfigProperty);

    class ShaderPassConfig : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ShaderPassConfig, Object);

    public:
        CORELIB_REFL_DECL_FIELD(RenderingType);
        ShaderPassRenderingType RenderingType{};

        CORELIB_REFL_DECL_FIELD(CullMode);
        CullMode CullMode{};

        CORELIB_REFL_DECL_FIELD(DepthTestEnable);
        bool DepthTestEnable{};

        CORELIB_REFL_DECL_FIELD(DepthWriteEnable);
        bool DepthWriteEnable{};

        CORELIB_REFL_DECL_FIELD(DepthCompareOp);
        CompareMode DepthCompareOp{};

        CORELIB_REFL_DECL_FIELD(StencilTestEnable);
        bool StencilTestEnable{};

        CORELIB_REFL_DECL_FIELD(ConstantProperties, new ListItemAttribute(cltypeof<ShaderPassConfigProperty>()));
        List_sp<ShaderPassConfigProperty_sp> ConstantProperties;

        CORELIB_REFL_DECL_FIELD(Properties, new ListItemAttribute(cltypeof<ShaderPassConfigProperty>()));
        List_sp<ShaderPassConfigProperty_sp> Properties;

        ShaderPassConfig()
        {
            init_sptr_member(ConstantProperties);
            init_sptr_member(Properties);
        }
    };
    CORELIB_DECL_SHORTSPTR(ShaderPassConfig);

    struct ShaderSourceData
    {
        struct ApiPlatform
        {
            string Config;
            hash_map<gfx::GFXShaderStageFlags, array_list<char>> Sources;
        };
        hash_map<gfx::GFXApi, ApiPlatform> ApiMaps;
    };

    std::iostream& ReadWriteStream(std::iostream& stream, bool write, ShaderSourceData& data);


    enum class EngineInputSemantic : int
    {
        POSITION = 0,
        NORMAL = 1,
        TANGENT = 2,
        BITANGENT = 3,
        COLOR = 4,
        TEXCOORD0 = 5,
        TEXCOORD1 = 6,
        TEXCOORD2 = 7,
        TEXCOORD3 = 8,
    };

    class Shader final : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Shader, AssetObject);
        CORELIB_CLASS_ATTR(new MenuItemCreateAssetAttribute);
    public:
        virtual void Serialize(AssetSerializer* s) override;

        static sptr<Shader> StaticCreate(string_view name, ShaderSourceData&& pass);

    public:
        Shader();

        virtual void OnDestroy() override;

        void ResetShaderSource(const ShaderSourceData& serData);
        const ShaderSourceData& GetSourceData() const { return m_shaderSource; }
        String_sp GetPassName() const { return m_passName; }

        ShaderPassConfig* GetConfig() const;
        void SetConfig(ShaderPassConfig_rsp value) { m_shaderConfig = value; }

        array_list<gfx::GFXApi> GetSupportedApi() const;
        bool HasSupportedApiData(gfx::GFXApi api) const;

        ShaderPassRenderingType GetRenderingType() const { return m_renderingType; }
    private:
        ShaderSourceData m_shaderSource;

        CORELIB_REFL_DECL_FIELD(m_passName);
        String_sp m_passName;

        CORELIB_REFL_DECL_FIELD(m_preDefines);
        List_sp<String_sp> m_preDefines;

        // CORELIB_REFL_DECL_FIELD(m_shaderConfig);
        ShaderPassConfig_sp m_shaderConfig;

        CORELIB_REFL_DECL_FIELD(m_renderingType);
        ShaderPassRenderingType m_renderingType{};
    public:

        #ifdef WITH_EDITOR

        Action<> OnAvailableChanged;
        std::mutex m_isAvailableMutex;
        bool m_isAvailable = false;

        #endif
    };
    DECL_PTR(Shader);



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
