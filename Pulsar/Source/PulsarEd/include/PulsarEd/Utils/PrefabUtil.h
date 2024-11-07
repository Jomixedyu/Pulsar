#pragma once
#include "Pulsar/Prefab.h"
#include "Pulsar/Scene.h"
#include "PulsarEd/Assembly.h"

namespace pulsared
{
    class PrefabUtil final
    {
    public:
        static RCPtr<Scene> NewSceneFromPrefab(RCPtr<Prefab> prefab);
        static void OpenPrefab(RCPtr<Prefab> prefab);
        static void ClosePrefabMode();
    };
}