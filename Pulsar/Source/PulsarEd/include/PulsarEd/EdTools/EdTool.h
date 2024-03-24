#pragma once
#include <PulsarEd/Assembly.h>

namespace pulsared
{
    class EditorWorld;
    class EdTool
    {
    public:
        virtual ~EdTool() = default;
        void Initialize(EditorWorld* world);
        virtual void Begin() {}
        virtual void Tick(float dt) {}
        virtual void End(){}
        [[always_inline]] EditorWorld* GetWorld() const noexcept { return m_world; }
    protected:
        EditorWorld* m_world{};
    };

} // namespace pulsared
