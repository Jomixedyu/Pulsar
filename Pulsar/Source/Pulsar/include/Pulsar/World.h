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

    class World
    {
    public: //static functions
        static World* Current();
        static World* Reset(std::unique_ptr<World>&& world);
        template<typename T>
        static T* Reset()
        {
            return static_cast<T*>(Reset(std::unique_ptr<World>(new T)));
        }
        static inline Action<> OnWorldChanged;


    public:
        World();
        virtual ~World();
    public:
        virtual void Tick(float dt);
        virtual void OnWorldBegin();
        virtual void OnWorldEnd();
        virtual void OnSceneLoading(ObjectPtr<Scene> scene);
        virtual void OnSceneUnloading(ObjectPtr<Scene> scene);


    public: // properties
        Ticker                      GetTicker() const { return m_ticker; }
        virtual CameraComponent_ref GetPreviewCamera();
        const ObjectPtr<Scene>&     GetScene(int index) const { return m_scenes[index]; }
        ObjectPtr<Scene>            GetFocusScene() const { return m_focusScene; }
        void                        SetFocusScene(ObjectPtr<Scene> scene);
        size_t                      GetSceneCount() const { return m_scenes.size(); }
        ObjectPtr<Scene>            GetPersistentScene() const { return m_scenes[0]; }


    public: // scene managment
        void ChangeScene(ObjectPtr<Scene> scene, bool clearPresistentScene = true);
        void LoadScene(ObjectPtr<Scene> scene);
        void UnloadScene(ObjectPtr<Scene> scene);
    private:
        void InitializePersistentScene();
        void UnloadAllScene(bool unloadPresistentScene = true);
    protected:
        virtual void OnLoadingPersistentScene(ObjectPtr<Scene> scene);
        virtual void OnUnloadingPersistentScene(ObjectPtr<Scene> scene);


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
        void         SetDefaultMaterial(Material_ref value) { m_defaultMaterial = value; }
        Material_ref GetDefaultMaterial() const { return m_defaultMaterial; }


    protected:
        Material_ref                          m_defaultMaterial;
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
    };
}