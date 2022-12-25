#pragma once
#include <Apatite/ObjectBase.h>

namespace apatite
{
    class World final
    {
    public:
        static World* Current();
        static World* Reset(World* world);
    public:
        void Tick(float dt);

    protected:
        virtual void OnWorldBegin();
        virtual void OnWorldEnd();
    private:
    };
}