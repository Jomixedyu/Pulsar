#pragma once
#include "ObjectBase.h"
#include "Components/Component.h"
#include "Rendering/RenderObject.h"

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
    protected:
        virtual void OnWorldBegin();
        virtual void OnWorldEnd();
    public:
        const ObjectPtr<Scene>& GetScene(int index) const { return m_scenes[index]; }
        size_t GetSceneCount() const { return m_scenes.size(); }
        ObjectPtr<Scene> GetPresistentScene() const { return m_scenes[0]; }

        void ChangeScene(ObjectPtr<Scene> scene, bool clearPresistentScene = true);


        const array_list<rendering::RenderObject*>& GetRenderObjects() const { return m_renderObjects; }
    private:
        void InitializePresistentScene();
    protected:
        array_list<rendering::RenderObject*>  m_renderObjects;
        array_list<ObjectPtr<Scene>>          m_scenes;
    };
}