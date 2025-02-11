#pragma once
#include "Assets/Material.h"
#include "CameraManager.h"
#include "Components/Component.h"
#include "ObjectBase.h"
#include "Rendering/RenderObject.h"
#include "SceneCaptureManager.h"
#include "Simulate.h"

namespace pulsar
{
    class Scene;
    class Material;

    constexpr uint32_t kRenderingDescriptorSpace_World = 1;

    class PhysicsWorld2D;
    class PhysicsWorld3D;
    class WorldSubsystem;
    class LightManager;

    class World
    {
    public: //static functions
        static World* Current();
        static World* Reset(std::unique_ptr<World>&& world);
        template<typename T>
        static T* Reset(string_view name)
        {
            return static_cast<T*>(Reset(std::unique_ptr<World>(new T{name})));
        }
        static inline Action<> OnWorldChanged;
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


        virtual void Tick(float dt);
        virtual void OnWorldBegin();
        virtual void OnWorldEnd();
        virtual void OnSceneLoading(RCPtr<Scene> scene);
        virtual void OnSceneUnloading(RCPtr<Scene> scene);
        virtual const char* GetWorldTypeName() const { return StaticWorldTypeName(); }
        static const char* StaticWorldTypeName() { return "World"; }

        const string& GetWorldName() const { return m_name; }

        virtual bool IsSelectedNode(const ObjectPtr<Node>& node) const { return false; }

        ObjectPtr<Node> FindNodeByName(string_view name, bool includeInactive = false) const;

        WorldSubsystem* GetSubsystem(Type* type) const;
        template <typename T>
        T* GetSubsystem() const
        {
            return static_cast<T*>(GetSubsystem(cltypeof<T>()));
        }

        int64_t AllocElementId(guid_t obj);
        void FreeElementId(int64_t id);
        guid_t FindElementId(int64_t id);

    public: // properties
        Ticker                      GetTicker() const { return m_ticker; }
        virtual ObjectPtr<CameraComponent> GetCurrentCamera();
        const RCPtr<Scene>&         GetScene(int index) const { return m_scenes[index]; }
        RCPtr<Scene>                GetFocusScene() const { return m_focusScene; }
        void                        SetFocusScene(RCPtr<Scene> scene);
        size_t                      GetSceneCount() const { return m_scenes.size(); }
        RCPtr<Scene>                GetResidentScene() const { return m_scenes[0]; }

        bool GetPlaying() const { return m_isPlaying; }
    public: // scene managment
        void ChangeScene(RCPtr<Scene> scene, bool clearResidentScene = true);
        void LoadScene(RCPtr<Scene> scene);
        void UnloadScene(RCPtr<Scene> scene);
    private:
        void InitializeResidentScene();
        void UnloadAllScene(bool unloadResidentScene = true);
    protected:
        virtual void OnLoadingResidentScene(RCPtr<Scene> scene);
        virtual void OnUnloadingResidentScene(RCPtr<Scene> scene);


    public: //rendering
        array_list<ObjectPtrBase>&      GetDeferredDestroyedQueue() { return m_deferredDestroyedQueue; }
        gfx::GFXDescriptorSet_sp        GetWorldDescriptorSet() const { return m_worldDescriptors; }
        const hash_set<rendering::RenderObject_sp>& GetRenderObjects() const { return m_renderObjects; }
        void            AddRenderObject(const rendering::RenderObject_sp& renderObject);
        void            RemoveRenderObject(rendering::RenderObject_rsp renderObject);
        CameraManager&        GetCameraManager() { return m_cameraManager; }
        SceneCaptureManager&  GetCaptureManager() { return m_captureManager; }
        GizmosManager&        GetGizmosManager() { return m_gizmosManager; }
        SimulateManager&      GetSimulateManager() { return m_simulateManager; }
        PhysicsWorld2D*       GetPhysicsWorld2D() const { return m_physicsWorld2D; }
        PhysicsWorld3D*       GetPhysicsWorld3D() const { return m_physicsWorld3D; }
        LightManager*         GetLightManager() const { return m_lightManager; }
    protected:
        void UpdateWorldCBuffer();
    protected:
        PhysicsWorld2D* m_physicsWorld2D = nullptr;
        PhysicsWorld3D* m_physicsWorld3D = nullptr;
        LightManager*   m_lightManager = nullptr;

        RCPtr<Material>                       m_defaultMaterial;
        hash_set<rendering::RenderObject_sp>  m_renderObjects;
        array_list<RCPtr<Scene>>              m_scenes;
        RCPtr<Scene>                          m_focusScene;
        CameraManager                         m_cameraManager;
        SceneCaptureManager                   m_captureManager;
        array_list<ObjectPtrBase>             m_deferredDestroyedQueue;
        SimulateManager                       m_simulateManager;

        gfx::GFXDescriptorSetLayout_sp m_worldDescriptorLayout;
        gfx::GFXBuffer_sp              m_worldDescriptorBuffer;
        gfx::GFXDescriptorSet_sp       m_worldDescriptors;

        GizmosManager m_gizmosManager;
        array_list<SPtr<class WorldSubsystem>> m_subsystems;

        hash_map<int64_t, guid_t> m_elementIdMap;

        Ticker   m_ticker{};
        float    m_totalTime = 0;
        string   m_name;
        bool     m_isPlaying{};
    };
}