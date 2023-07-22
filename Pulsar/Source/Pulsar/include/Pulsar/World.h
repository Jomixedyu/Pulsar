#pragma once
#include <Pulsar/ObjectBase.h>
#include "Components/Component.h"
namespace pulsar
{
    class Scene;
    class Material;



    class World
    {
    public:
        static World* Current();
        static World* Reset(World* world);
        static inline Action<> OnWorldChanged;
    public:
        World();
    public:
        virtual void Tick(float dt);
    protected:
        virtual void OnWorldBegin();
        virtual void OnWorldEnd();
    public:
        const sptr<Scene>& GetScene(int index) const { return m_scenes[index]; }
        const sptr<Scene>& GetPresistentScene() const { return m_scenes[0]; }

        void ChangeScene(sptr<Scene> scene, bool clearPresistentScene = true);
        const array_list<class IRenderObject*>& GetRenderObjects() const { return m_renderObjects; }
    protected:
        array_list<class IRenderObject*> m_renderObjects;
        array_list<sptr<Scene>> m_scenes;
    };
}