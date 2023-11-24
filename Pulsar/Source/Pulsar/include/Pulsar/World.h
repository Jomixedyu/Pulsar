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

    class World
    {
    public:
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

        virtual CameraComponent_ref GetPreviewCamera()
        {
            return GetCameraManager().GetMainCamera();
        }

    protected:
        virtual void OnWorldBegin();
        virtual void OnWorldEnd();
        virtual void OnSceneLoading(ObjectPtr<Scene> scene);
        virtual void OnSceneUnloading(ObjectPtr<Scene> scene);
    public:
        const ObjectPtr<Scene>& GetScene(int index) const { return m_scenes[index]; }
        size_t GetSceneCount() const { return m_scenes.size(); }
        ObjectPtr<Scene> GetPresistentScene() const { return m_scenes[0]; }

        void ChangeScene(ObjectPtr<Scene> scene, bool clearPresistentScene = true);
        void LoadScene(ObjectPtr<Scene> scene);
        void UnloadScene(ObjectPtr<Scene> scene);

        array_list<ObjectPtrBase>& GetDeferredDestroyedQueue() { return m_deferredDestroyedQueue; }

        const hash_set<rendering::RenderObject_sp>& GetRenderObjects() const { return m_renderObjects; }
        void AddRenderObject(const rendering::RenderObject_sp renderObject);
        void RemoveRenderObject(rendering::RenderObject_rsp renderObject);

        CameraManager& GetCameraManager() { return m_cameraManager; }
    private:
        void InitializePresistentScene();
        void UnloadAllScene(bool unloadPresistentScene = true);
    protected:
        virtual void OnLoadingPresistentScene(ObjectPtr<Scene> scene);
        virtual void OnUnloadingPresistentScene(ObjectPtr<Scene> scene);
    public:
        void SetDefaultMaterial(Material_ref value) { m_defaultMaterial = value; }
        Material_ref GetDefaultMaterial() const { return m_defaultMaterial; }
    protected:
        Material_ref                          m_defaultMaterial;
        hash_set<rendering::RenderObject_sp>  m_renderObjects;
        array_list<ObjectPtr<Scene>>          m_scenes;
        CameraManager                         m_cameraManager;
        array_list<ObjectPtrBase>             m_deferredDestroyedQueue;

    };
}