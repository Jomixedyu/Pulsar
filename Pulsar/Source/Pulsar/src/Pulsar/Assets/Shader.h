#pragma once

#include "Pulsar/BuiltinAsset.h"

#include <Pulsar/AssetObject.h>
#include <Pulsar/Rendering/ShaderConfig.h>

namespace pulsar
{
    enum class EngineInputSemantic : int
    {
        POSITION  = 0,
        NORMAL    = 1,
        TANGENT   = 2,
        // 3 保留（原 BITANGENT，已移除，w 分量并入 Tangent.w）
        COLOR     = 4,
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

        static RCPtr<Shader> StaticCreate(string_view name);

        SPtr<ShaderConfig> GetConfig() const { return m_config; }

    public:
        Shader();

        virtual void OnDestroy() override;

    protected:
        CORELIB_REFL_DECL_FIELD(m_config)
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
