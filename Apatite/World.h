#pragma once
#include <Apatite/ObjectBase.h>

namespace apatite
{
    class World
    {
    public:
        static World* Current();
        static World* Reset(World* world);
    public:
        virtual void Tick(float dt);
        sptr<class Scene> scene;
    protected:
        virtual void OnWorldBegin();
        virtual void OnWorldEnd();
    private:
    };
}