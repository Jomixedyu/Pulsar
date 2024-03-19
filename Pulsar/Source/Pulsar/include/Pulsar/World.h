#pragma once
#include "ObjectBase.h"
#include "Components/Component.h"
#include "Rendering/RenderObject.h"
#include "CameraManager.h"
#include "Assets/Material.h"

namespace pulsar
{
    class Scene;
    class Material;

    constexpr uint32_t kRenderingDescriptorSpace_World = 1;

    struct WorldRenderBufferData
    {
        Vector4f WorldSpaceLightVector;
        Vector4f WorldSpaceLightColor; // w is intensity
        float TotalTime;
        float DeltaTime;
        Vector2f _Padding0;
        Vector4f SkyLightColor; // w is intensity
    };

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
        virtual void Tick(float dt);
        virtual void OnWorldBegin();
        virtual void OnWorldEnd();
        virtual void OnSceneLoading(ObjectPtr<Scene> scene);
        virtual void OnSceneUnloading(ObjectPtr<Scene> scene);
        virtual const char* GetWorldTypeName() const { return StaticWorldTypeName(); }
        static const char* StaticWorldTypeName() { return "World"; }

        const string& GetWorldName() const { return m_name; }

    public: // properties
        Ticker                      GetTicker() const { return m_ticker; }
        virtual ObjectPtr<CameraComponent> GetPreviewCamera();
        const ObjectPtr<Scene>&     GetScene(int index) const { return m_scenes[index]; }
        ObjectPtr<Scene>            GetFocusScene() const { return m_focusScene; }
        void                        SetFocusScene(ObjectPtr<Scene> scene);
        size_t                      GetSceneCount() const { return m_scenes.size(); }
        ObjectPtr<Scene>            GetResidentScene() const { return m_scenes[0]; }


    public: // scene managment
        void ChangeScene(ObjectPtr<Scene> scene, bool clearResidentScene = true);
        void LoadScene(ObjectPtr<Scene> scene);
        void UnloadScene(ObjectPtr<Scene> scene);
    private:
        void InitializeResidentScene();
        void UnloadAllScene(bool unloadResidentScene = true);
    protected:
        virtual void OnLoadingResidentScene(ObjectPtr<Scene> scene);
        virtual void OnUnloadingResidentScene(ObjectPtr<Scene> scene);


    public: //rendering
        array_list<ObjectPtrBase>&      GetDeferredDestroyedQueue() { return m_deferredDestroyedQueue; }
        gfx::GFXDescriptorSet_sp        GetWorldDescriptorSet() const { return m_worldDescriptors; }
        const hash_set<rendering::RenderObject_sp>& GetRenderObjects() const { return m_renderObjects; }
        void            AddRenderObject(const rendering::RenderObject_sp renderObject);
        void            RemoveRenderObject(rendering::RenderObject_rsp renderObject);
        CameraManager&  GetCameraManager() { return m_cameraManager; }
    protected:
        void UpdateWorldCBuffer();
    public:
        void         SetDefaultMaterial(RCPtr<Material> value) { m_defaultMaterial = value; }
        RCPtr<Material> GetDefaultMaterial() const { return m_defaultMaterial; }


    protected:
        RCPtr<Material>                       m_defaultMaterial;
        hash_set<rendering::RenderObject_sp>  m_renderObjects;
        array_list<ObjectPtr<Scene>>          m_scenes;
        ObjectPtr<Scene>                      m_focusScene;
        CameraManager                         m_cameraManager;
        array_list<ObjectPtrBase>             m_deferredDestroyedQueue;

        gfx::GFXDescriptorSetLayout_sp m_worldDescriptorLayout;
        gfx::GFXBuffer_sp              m_worldDescriptorBuffer;
        gfx::GFXDescriptorSet_sp       m_worldDescriptors;

        Ticker m_ticker{};
        float  m_totalTime = 0;
        string m_name;
    };
}