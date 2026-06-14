#pragma once

namespace pulsar::rendering
{
    // Base class for all render-thread-exclusive proxies (primitive / view / light).
    // A proxy is the render-thread mirror of a game-side RenderComponent. It is created
    // on the game thread, ownership is transferred to the render thread via the command
    // queue, and all access after registration happens on the render thread.
    // Resource lifecycle hooks run on the render thread inside RenderScene.
    class RenderProxy
    {
    public:
        virtual ~RenderProxy() = default;

        // Called on the render thread when the proxy is added to / removed from the scene.
        virtual void OnCreateResource() {}
        virtual void OnDestroyResource() {}
    };
}
