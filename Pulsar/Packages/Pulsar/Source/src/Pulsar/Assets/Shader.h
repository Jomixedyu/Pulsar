#pragma once

#include "Pulsar/BuiltinAsset.h"

#include <Pulsar/AssetObject.h>
#include <Pulsar/IGPUResource.h>
#include <Pulsar/Rendering/ShaderConfig.h>

namespace pulsar
{
    class ShaderProxy;

    enum class EngineInputSemantic : int
    {
        POSITION     = 0,
        NORMAL       = 1,
        TANGENT      = 2,
        // 3 保留（原 BITANGENT，已移除，w 分量并入 Tangent.w）
        COLOR        = 4,
        TEXCOORD0    = 5,
        TEXCOORD1    = 6,
        TEXCOORD2    = 7,
        TEXCOORD3    = 8,
        // TEXCOORD4~12 保留给更多 UV 套数
        BLENDINDICES = 13, // 骨骼索引 (uint4)
        BLENDWEIGHT  = 14, // 骨骼权重 (float4)
    };

    class Shader final : public AssetObject, public IGPUResource
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Shader, AssetObject);
        CORELIB_CLASS_ATTR(
            new CreateAssetAttribute(BuiltinAsset::Shader_Missing),
            new AssetIconAttribute("PulsarEd/Icons/shader.png")
            );
    public:
        virtual void Serialize(AssetSerializer* s) override;
        void PostEditChange(FieldInfo* info) override;

        static RCPtr<Shader> StaticCreate(string_view name);

        SPtr<ShaderConfig> GetConfig() const { return m_config; }

    public:
        Shader();

        virtual void OnDestroy() override;

        // The shader's render-thread proxy: created lazily by its first user
        // (material / render feature), removed when the asset is destroyed.
        bool EnsureGPUResource() override;
        void DestroyGPUResource() override;
        bool IsCreatedGPUResource() const override { return m_isCreatedGPUResource; }

        const std::shared_ptr<ShaderProxy>& GetRenderProxy() const { return m_renderProxy; }
        // Hot-reload (.hlsl): drop all variants so they recompile.
        void ReloadSceneProxy();

    protected:
        CORELIB_REFL_DECL_FIELD(m_config)
        // NOTE: mutable in the editor (reflected, edited in place by the property
        // panel). Consumers that cross threads must hold a Clone(), not this SPtr.
        SPtr<ShaderConfig> m_config;

        std::shared_ptr<ShaderProxy> m_renderProxy;
        bool m_isCreatedGPUResource = false;
        // Set by Serialize/PostEditChange; consumed by EnsureGPUResource to push
        // a fresh config snapshot exactly once per change.
        bool m_configDirty = false;
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
