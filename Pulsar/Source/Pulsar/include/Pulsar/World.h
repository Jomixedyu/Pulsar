#pragma once
#include <Pulsar/ObjectBase.h>

namespace pulsar
{
    class Scene;

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
        const sptr<Scene>& GetScene() const { return m_scene; }
        void ChangeScene(sptr<Scene> scene);
    protected :
        sptr<Scene> m_scene;
    };
}