#pragma once
#include <PulsarEd/Assembly.h>

namespace pulsared
{

    class EdTool
    {
    public:
        virtual ~EdTool() = default;
        void Initialize(World* world);
        virtual void Begin() {}
        virtual void Tick(float dt) {}
        virtual void End(){}
    protected:
        World* m_world{};
    };

} // namespace pulsared
