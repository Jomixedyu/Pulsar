#pragma once

#include <gfx/GFXDescriptorSet.h>
#include <gfx/GFXHandle.h>
#include <Pulsar/Rendering/ShaderPropertySheet.h>
#include <Pulsar/Rendering/ShaderPropertyRenderData.h>
#include <Pulsar/Rendering/ShaderInstance.h>
#include <Pulsar/Rendering/MaterialProxy.h>
#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/Texture.h>
#include <Pulsar/Rendering/ObjectPropertyOverride.h>
#include <Pulsar/Meta/ToolFunctionAttribute.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace pulsar
{
    class Material final : public AssetObject, public IGPUResource
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Material, AssetObject);
        CORELIB_CLASS_ATTR(
            new CreateAssetAttribute("Pulsar/Materials/Error"),
            new AssetIconAttribute("PulsarEd/Icons/material.png")
            );

    public:
        Material();

        static RCPtr<Material> StaticCreate(const RCPtr<Shader>& shader, string_view name = {});

        virtual void Serialize(AssetSerializer* s) override;
    public:
        virtual bool CreateGPUResource() override;
        virtual void DestroyGPUResource() override;
        virtual bool IsCreatedGPUResource() const override;

        void OnCollectAssetDependencies(array_list<jxcorlib::guid_t> &deps) override;
        void GetSubscribeObserverHandles(array_list<ObjectHandle>& out) override;

    protected:
        void OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg) override;
    public:
        // Parameter accessors (operate on m_sheet)
        void SetIntScalar(const index_string& name, int value);
        void SetFloat(const index_string& name, float value);
        void SetTexture(const index_string& name, const RCPtr<Texture>& value);
        void SetVector4(const index_string& name, const Vector4f& value);
        int GetIntScalar(const index_string& name);
        float GetScalar(const index_string& name);
        Vector4f GetVector4(const index_string& name);
        RCPtr<Texture> GetTexture(const index_string& name);

        // User-triggered: upload dirty parameters to the render proxy.
        // Call this after modifying material parameters (SetFloat / SetTexture / etc.).
        void SubmitParameters(bool force = false);

        // Render-thread mirror of this material's render state (1:1). Created in CreateGPUResource.
        // Render-side code (mesh batches, post-process passes) references this proxy instead of
        // the Material so the render thread never holds an RCPtr<Material>.
        const std::shared_ptr<MaterialProxy>& GetRenderProxy() const { return m_renderProxy; }

    public:
        RCPtr<Shader> GetShader() const;
                void SetShader(RCPtr<Shader> value);
        void ApplyShaderDefaults();

        void SetGraphicsPipelineOverride(const SPtr<ShaderConfigGraphicsPipeline>& value)
        {
            m_graphicsPipelineOverride = value;
            EnqueueProxyStateUpdate();
        }

        void SetGraphicsPipelineOverrideFields(const SPtr<ObjectPropertyOverride>& value)
        {
            m_graphicsPipelineOverrideFields = value;
            EnqueueProxyStateUpdate();
        }

        const std::vector<std::string>& GetActiveFeatures() const { return m_activeFeatures; }
        void SetActiveFeatures(std::vector<std::string> features);

        ShaderPropertySheet& GetSheet() { return m_sheet; }
        const ShaderPropertySheet& GetSheet() const { return m_sheet; }

        ShaderPassRenderQueueType GetQueue() const { return m_queue; }
        void SetQueue(ShaderPassRenderQueueType value) { if (m_queue == value) return; m_queue = value; EnqueueProxyStateUpdate(); }

        void InvalidateGraphicsPipelineCache() { EnqueueProxyStateUpdate(); }

        SPtr<ShaderConfigGraphicsPipeline> GetGraphicsPipelineOverride() const { return m_graphicsPipelineOverride; }

        SPtr<ObjectPropertyOverride> GetGraphicsPipelineOverrideFields() const { return m_graphicsPipelineOverrideFields; }

        CORELIB_REFL_DECL_METHOD(SetOpaqueOverride, new ToolFunctionAttribute("Set Opaque"));
        void SetOpaqueOverride();

        CORELIB_REFL_DECL_METHOD(SetTranslucentOverride, new ToolFunctionAttribute("Set Translucent"));
        void SetTranslucentOverride();

        CORELIB_REFL_DECL_METHOD(RestorePipelineDefaults, new ToolFunctionAttribute("Restore Defaults"));
        void RestorePipelineDefaults();

        CORELIB_REFL_DECL_METHOD(RebuildOverrideFields, new ToolFunctionAttribute("Rebuild Fields"));
        void RebuildOverrideFields();

        Action<> OnShaderChanged;

    protected:
        void PostEditChange(FieldInfo* info) override;

    private:
        // 【游戏线程】从 m_sheet 重建渲染线程专用快照 m_renderData。
        void RebuildRenderData();
        // 【游戏线程】把 shader/feature 快照入队，喂给渲染线程的 proxy（清 bindings，触发重编译）。
        void EnqueueProxyShaderUpdate();
        // 【游戏线程】把 queue/GP override 等轻量状态入队，喂给渲染线程的 proxy（不清 bindings）。
        void EnqueueProxyStateUpdate();
        // 【游戏线程】把当前参数快照入队，喂给渲染线程的 proxy。
        void EnqueueProxyRenderData();

    private:
        CORELIB_REFL_DECL_FIELD(m_shader);
        RCPtr<Shader> m_shader;

        CORELIB_REFL_DECL_FIELD(m_queue);
        ShaderPassRenderQueueType m_queue = ShaderPassRenderQueueType::Opaque;

        CORELIB_REFL_DECL_FIELD(m_graphicsPipelineOverride);
        SPtr<ShaderConfigGraphicsPipeline> m_graphicsPipelineOverride;

        CORELIB_REFL_DECL_FIELD(m_graphicsPipelineOverrideFields);
        SPtr<ObjectPropertyOverride> m_graphicsPipelineOverrideFields;

        ShaderPropertySheet m_sheet;
        // 游戏线程从 m_sheet 解析的渲染线程专用快照（纹理已解析为 GPU 句柄、常量纯值）。
        // 该快照经渲染队列喂给 m_renderProxy；渲染线程只消费 proxy 内的拷贝。
        ShaderPropertyRenderData m_renderData;
        std::vector<std::string> m_activeFeatures;

        // 渲染线程独占镜像（1:1）。承载所有 (pass×interface×feature) 变体 binding 与 GPU 资源。
        std::shared_ptr<MaterialProxy> m_renderProxy;

        bool m_createdGpuResource = false;
        bool m_isDirtyParameter{};
    };

} // namespace pulsar
