#pragma once
#include "ObjectBase.h"
#include <CoreLib.Serialization/ObjectSerializer.h>

namespace pulsar
{
    class Scene;

    class SceneObject : public ObjectBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SceneObject, ObjectBase);
    public:

        void SceneObjectConstruct(guid_t guid = {});

        guid_t GetSceneObjectGuid() const { return m_sceneObjectGuid; }

        virtual void GetDependenciesAsset(array_list<guid_t>& deps) const {}

    protected:
        guid_t m_sceneObjectGuid;
    public:
        guid_t m_sourceSceneObjectGuid; //in prefab
    };


    class ISceneObjectFinder
    {
    public:
        virtual ~ISceneObjectFinder() = default;
        virtual ObjectPtr<SceneObject> FindSceneObject(guid_t sceneObjId) const = 0;
        virtual void AddSceneObjectToFinder(const ObjectPtr<SceneObject>& obj) = 0;
    };

    struct SceneObjectSerializer
    {
        SceneObjectSerializer(ser::VarientRef obj, bool isWrite, bool editorData,
                              ISceneObjectFinder* sceneObjectFinder = nullptr)
            : Object(std::move(obj)),
              IsWrite(isWrite),
              HasEditorData(editorData),
              SceneObjectFinder(sceneObjectFinder)
        {
        }

        SceneObjectSerializer(const SceneObjectSerializer&) = delete;
        SceneObjectSerializer(SceneObjectSerializer&&) = delete;
    public:
        ISceneObjectFinder* SceneObjectFinder;
        ser::VarientRef Object;
        const bool IsWrite;
        const bool HasEditorData;
    };

}