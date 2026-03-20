#pragma once

#include "Pulsar/BuiltinAsset.h"

#include "Pulsar/MaterialParameterValue.h"
#include "gfx/GFXGpuProgram.h"

#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Texture.h>
#include <Pulsar/IGPUResource.h>
#include <Pulsar/ObjectBase.h>
#include <Pulsar/Rendering/ShaderConfig.h>
#include <Pulsar/Rendering/Types.h>
#include <functional>
#include <gfx/GFXApi.h>
#include <mutex>

namespace pulsar
{
    struct ShaderSourceData
    {

        struct ApiPlatform
        {
            string Config;
            hash_map<gfx::GFXGpuProgramStageFlags, array_list<char>> Sources;
        };
        hash_map<gfx::GFXApi, ApiPlatform> ApiMaps;
    };
}

namespace pulsar
{
    class Texture2D;


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
        CORELIB_CLASS_ATTR(new CreateAssetAttribute(BuiltinAsset::Shader_Missing));
    public:
        virtual void Serialize(AssetSerializer* s) override;

        static RCPtr<Shader> StaticCreate(string_view name, ShaderSourceData&& pass);

        void OnInstantiateAsset(AssetObject* obj) override;

    public:
        Shader();

        virtual void OnDestroy() override;

        void ResetShaderSource(ShaderSourceData&& serData);
        const ShaderSourceData& GetSourceData() const { return m_shaderSource; }
        String_sp GetPassName() const { return m_passName; }

        auto GetConfig() const { return m_config; }

        array_list<gfx::GFXApi> GetSupportedApi() const;
        bool HasSupportedApiData(gfx::GFXApi api) const;

        array_list<index_string> GetPropertyNames() const;
        const MaterialParameterInfo* GetPropertyInfo(index_string name) const;

        size_t GetConstantBufferSize() const noexcept { return m_constantBufferSize; }
        auto& GetFeatureOptions() noexcept { return m_featureOptions; }

        bool IsReady() const { return m_isReady; }
        void SetReady(bool b);
    protected:
        void Initialize();
    private:
        ShaderSourceData m_shaderSource;

        CORELIB_REFL_DECL_FIELD(m_passName);
        String_sp m_passName;

        CORELIB_REFL_DECL_FIELD(m_preDefines, new ListItemAttribute(cltypeof<String>()));
        List_sp<String_sp> m_preDefines;

        array_list<string> m_featureOptions;

        size_t m_compiledHash{};

        // runtime data
        hash_map<index_string, MaterialParameterInfo> m_propertyInfo;
        size_t m_constantBufferSize{};

        bool m_isReady{};
        
        SPtr<ShaderConfig> m_config; //read only
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
