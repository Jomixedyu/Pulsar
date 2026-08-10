#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/Rendering/RenderGraph/ScriptableCaptureRenderer.h>
#include <gfx/GFXBuffer.h>
#include <memory>
#include <typeindex>
#include <utility>
#include <unordered_map>

namespace pulsar
{
    class RenderFrameData
    {
    public:
        template<typename T>
        T* Get()
        {
            auto it = m_items.find(std::type_index(typeid(T)));
            if (it == m_items.end())
                return nullptr;
            return static_cast<T*>(it->second.get());
        }

        template<typename T>
        const T* Get() const
        {
            auto it = m_items.find(std::type_index(typeid(T)));
            if (it == m_items.end())
                return nullptr;
            return static_cast<const T*>(it->second.get());
        }

        template<typename T>
        T& GetOrCreate()
        {
            auto key = std::type_index(typeid(T));
            auto it = m_items.find(key);
            if (it == m_items.end())
            {
                auto item = MakeItem<T>();
                auto* ptr = static_cast<T*>(item.get());
                m_items.emplace(key, std::move(item));
                return *ptr;
            }
            return *static_cast<T*>(it->second.get());
        }

        template<typename T>
        T& Set(T data)
        {
            auto item = MakeItem<T>(std::move(data));
            auto* ptr = static_cast<T*>(item.get());
            m_items.insert_or_assign(std::type_index(typeid(T)), std::move(item));
            return *ptr;
        }

    private:
        template<typename T, typename... TArgs>
        static std::unique_ptr<void, void(*)(void*)> MakeItem(TArgs&&... args)
        {
            return {
                new T(std::forward<TArgs>(args)...),
                [](void* p) { delete static_cast<T*>(p); }
            };
        }

        std::unordered_map<std::type_index, std::unique_ptr<void, void(*)(void*)>> m_items;
    };

    struct ViewFrameData
    {
        const SceneViewData* ViewData = nullptr;
        SceneView* View = nullptr;
        uint64_t FrameIndex = 0;
    };

    struct SceneFrameData
    {
        RenderScene* Scene = nullptr;
    };

    struct GpuFrameData
    {
        gfx::GFXBuffer* CameraBuffer = nullptr;
        gfx::GFXBuffer* WorldBuffer = nullptr;
        gfx::GFXBuffer* LightBuffer = nullptr;
        gfx::GFXBuffer* RenderObjectBuffer = nullptr;
    };

    struct SceneTargetFrameData
    {
        RGTextureHandle Target;
    };

    struct OpaqueColorFrameData
    {
        RGTextureHandle Color;
    };

    struct PostProcessFrameData
    {
        RGTextureHandle FinalTarget;
        RGTextureHandle ActiveColor;
        RGTextureHandle PingPongA;
        RGTextureHandle PingPongB;

        RGTextureHandle AcquireTarget() const
        {
            return ActiveColor == PingPongA ? PingPongB : PingPongA;
        }

        void PushColor(RGTextureHandle handle)
        {
            ActiveColor = handle;
        }
    };


    inline RenderCaptureContext MakeRenderCaptureContext(const RenderFrameData& frameData)
    {
        RenderCaptureContext ctx{};
        if (auto* view = frameData.Get<ViewFrameData>())
        {
            ctx.view = view->ViewData;
            ctx.viewProxy = view->View;
            ctx.frameIndex = view->FrameIndex;
        }
        if (auto* scene = frameData.Get<SceneFrameData>())
        {
            ctx.scene = scene->Scene;
        }
        return ctx;
    }
    class RenderFeature
    {
    public:
        virtual ~RenderFeature() = default;

        virtual void Initialize() {}
        virtual void Destroy() {}

        virtual void OnRecord(RenderGraph& graph, RenderFrameData& frameData) = 0;
    };

} // namespace pulsar
