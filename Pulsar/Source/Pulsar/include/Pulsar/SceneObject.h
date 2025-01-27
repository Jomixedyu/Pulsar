#pragma once
#include "ObjectBase.h"

namespace pulsar
{
    class Scene;

    class SceneObject : public ObjectBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SceneObject, ObjectBase);
    public:
        virtual void BeginScene(const ObjectPtr<Scene>& scene) {}
        virtual void EndScene() {}
    protected:
        guid_t m_sceneObjectId;
    };

    class ISceneObjectFinder
    {
    public:
        virtual ~ISceneObjectFinder() = default;
        virtual ObjectHandle Find(guid_t sceneObjId) = 0;
    };

    template <typename T>
    struct SceneObjectWeakPtr
    {
        ObjectPtr<SceneObject> Load(ISceneObjectFinder* finder)
        {
            return finder->Find(m_ref);
        }
        guid_t m_ref;
    };
}