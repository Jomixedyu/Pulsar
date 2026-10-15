#pragma once
#include "Component.h"
#include <Pulsar/Rendering/SceneProxy.h>

namespace pulsar
{
    // Base class for all game-side components that own a render-thread SceneProxy
    // (mesh renderers, scene captures, lights). Unifies the proxy lifecycle:
    //   BeginComponent  -> CreateSceneProxy + World::AddSceneProxy   (enqueue Add)
    //   EndComponent    -> World::RemoveSceneProxy                    (enqueue Remove)
    //   setter changes  -> MarkRenderStateDirty                        (register dirty)
    //   end of Tick     -> ResolveRenderStateDirty                    (snapshot + enqueue update)
    class RenderComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RenderComponent, Component);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
        friend class World;
    public:
        void BeginComponent() override;
        void EndComponent() override;

        // Register this component into the world's end-of-frame dirty render-state list.
        // Setters that change render-relevant state call this; ResolveDirtyRenderStates()
        // later calls ResolveRenderStateDirty() on the game thread.
        void MarkRenderStateDirty();

    protected:
        // Create the render-thread proxy mirror for this component. Called in
        // BeginComponent. Return nullptr to skip (no proxy registered this lifetime).
        virtual SPtr<rendering::SceneProxy> CreateSceneProxy() = 0;

        // Snapshot render-relevant data and enqueue the write to the render thread.
        // Called during the extraction phase (end of World::Tick) for dirty components.
        virtual void ResolveRenderStateDirty() {}

        SPtr<rendering::SceneProxy> m_proxy;
        bool m_renderStateDirty = false;
    };
    DECL_PTR(RenderComponent);
}
