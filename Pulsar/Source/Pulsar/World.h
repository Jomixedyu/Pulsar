#pragma once
#include <Pulsar/ObjectBase.h>

namespace pulsar
{
    class World
    {
    public:
        static World* Current();
        static World* Reset(World* world);
        static inline Action<> OnWorldChanged;
    public:
        sptr<class Scene> scene;
    public:
        virtual void Tick(float dt);

        void Draw();
    protected:
        virtual void OnWorldBegin();
        virtual void OnWorldEnd();
    private:
    };
}