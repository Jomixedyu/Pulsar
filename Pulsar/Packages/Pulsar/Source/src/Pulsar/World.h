#pragma once
#include "Assets/Material.h"
#include "CameraManager.h"
#include "Components/Component.h"
#include "ObjectBase.h"
#include "Rendering/RenderObject.h"
#include "Rendering/RenderScene.h"
#include "SceneCaptureManager.h"
#include "SelectionSet.h"
#include "Simulate.h"

namespace uinput { struct InputEvent; }

namespace pulsar
{
    class NodeCollection;
    class Material;
    class RenderComponent;
    class SceneCaptureComponent;

    class PhysicsWorld2D;
    class PhysicsWorld3D;
    class WorldSubsystem;
    struct InputContext;

    class World
    {
    public: //static functions
        static const hash_set<World*>& GetAllWorlds();

    public:
        explicit World(string_view name);
        virtual ~World();
    public:

        void OnDuplicated(World* target);

        virtual void BeginPlay();
        virtual void EndPlay();

        virtual void BeginSimulate();
        virtual void EndSimulate();

        bool IsSimulating() const { return m_isSimulating; }

        void CameraFocusNode(Node* node);
        virtual void Tick(float dt);
        virtual void OnWorldBegin();
        virtual void OnWorldEnd();
        virtual void OnSceneLoading(RCPtr<NodeCollection> scene);
        virtual void OnSceneUnloading(RCPtr<NodeCollection> scene);
        virtual const char* GetWorldTypeName() const { return StaticWorldTypeName(); }
        static const char* StaticWorldTypeName() { return "World"; }

        const string& GetWorldName() const { return m_name; }

        bool IsSelectedNode(const ObjectPtr<Node>& node) const;
        SelectionSet<Node>& GetSelection() { return m_selection; }
        const SelectionSet<Node>& GetSelection() const { return m_selection; }

        ObjectPtr<Node> FindNodeByName(string_view name, bool includeInactive = false) const;

        WorldSubsystem* GetSubsystem(Type* type) const;
        template <typename T>
        T* GetSubsystem() const
        {
            return static_cast<T*>(GetSubsystem(cltypeof<T>()));
        }


    public: // properties
        Ticker                      GetTicker() const { return m_ticker; }
        float                       GetTotalTime() const { return m_totalTime; }
        virtual ObjectPtr<CameraComponent> GetCurrentCamera();
        const RCPtr<NodeCollection>& GetScene(int index) const { return m_scenes[index]; }
        RCPtr<NodeCollection>        GetFocusScene() const { return m_focusScene; }
        void                         SetFocusScene(RCPtr<NodeCollection> scene);
        size_t                       GetSceneCount() const { return m_scenes.size(); }
        RCPtr<NodeCollection>        GetResidentScene() const { return m_scenes[0]; }

        bool GetPlaying() const { return m_isPlaying; }
        bool GetPaused() const { return m_isPaused; }
        void SetPaused(bool paused) { m_isPaused = paused; }

        InputContext* GetInputContext() const { return m_inputContext; }

        void BeginInputFrame();
        void ProcessInputEvent(const uinput::InputEvent& e);

    public: // scene managment
        void ChangeScene(RCPtr<NodeCollection> scene, bool clearResidentScene = true);
        void LoadScene(RCPtr<NodeCollection> scene);
        void UnloadScene(RCPtr<NodeCollection> scene);
    private:
        void InitializeResidentScene();
        void UnloadAllScene(bool unloadResidentScene = true);
    protected:
        virtual void OnLoadingResidentScene(RCPtr<NodeCollection> scene);
        virtual void OnUnloadingResidentScene(RCPtr<NodeCollection> scene);


    public: //rendering
        array_list<ObjectPtrBase>&      GetDeferredDestroyedQueue() { return m_deferredDestroyedQueue; }
        RenderScene*    GetRenderScene() const { return m_renderScene.get(); }

        // Raw render objects without an owning RenderComponent (gizmos, editor grids,
        // light gizmos). Enqueued straight to the render thread.
        void            AddRenderObject(const rendering::RenderObject_sp& ro);
        void            RemoveRenderObject(const rendering::RenderObject_sp& ro);

        // Component-owned proxies. RegisterProxy creates the RenderProxy from the
        // component (RenderComponent::CreateRenderProxy), stores it on the component,
        // and adds it to the render scene. UnregisterProxy removes + drops it.
        void            RegisterProxy(RenderComponent* comp);
        void            UnregisterProxy(RenderComponent* comp);

        // Per-view snapshot update. The caller passes the target view proxy directly
        // (the component caches it); the render thread writes Data with no lookup. Enqueued.
        void            UpdateSceneView(const SPtr<SceneView>& view, SceneViewData data);

        // Per-frame dirty extraction. Components mark themselves dirty when render state
        // changes; SyncRenderProxies() drains the pending list once per frame.
        void            MarkProxyDirty(RenderComponent* comp);
        void            UnmarkProxyDirty(RenderComponent* comp);
        void            SyncRenderProxies();

        CameraManager&        GetCameraManager() { return m_cameraManager; }
        SceneCaptureManager&  GetCaptureManager() { return m_captureManager; }
        GizmosManager&        GetGizmosManager() { return m_gizmosManager; }
        SimulateManager&      GetSimulateManager() { return m_simulateManager; }
        PhysicsWorld2D*       GetPhysicsWorld2D() const { return m_physicsWorld2D; }
        PhysicsWorld3D*       GetPhysicsWorld3D() const { return m_physicsWorld3D; }
    protected:
        void TeardownRenderScene();

        SelectionSet<Node> m_selection;
    protected:
        PhysicsWorld2D* m_physicsWorld2D = nullptr;
        PhysicsWorld3D* m_physicsWorld3D = nullptr;
        RCPtr<Material>                       m_defaultMaterial;
        std::unique_ptr<RenderScene>          m_renderScene = std::make_unique<RenderScene>();
        array_list<RenderComponent*>          m_pendingProxyUpdates;
        array_list<RCPtr<NodeCollection>>     m_scenes;
        RCPtr<NodeCollection>                 m_focusScene;
        CameraManager                         m_cameraManager;
        SceneCaptureManager                   m_captureManager;
        array_list<ObjectPtrBase>             m_deferredDestroyedQueue;
        SimulateManager                       m_simulateManager;

        GizmosManager m_gizmosManager;
        array_list<SPtr<class WorldSubsystem>> m_subsystems;


        Ticker   m_ticker{};
        float    m_totalTime = 0;
        string   m_name;
        bool     m_isPlaying{};
        bool     m_isSimulating = false;
        bool     m_isPaused = false;
        InputContext* m_inputContext;
    };
}