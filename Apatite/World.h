#pragma once
#include <Apatite/ObjectBase.h>

namespace apatite
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

    protected:
        virtual void OnWorldBegin();
        virtual void OnWorldEnd();
    private:
    };
}