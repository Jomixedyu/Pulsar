#pragma once
#include "Pulsar/Prefab.h"
#include "Pulsar/Scene.h"
#include "PulsarEd/Assembly.h"

namespace pulsared
{
    class PrefabUtil final
    {
    public:
        static Scene_ref NewSceneFromPrefab(Prefab_ref prefab);
        static void OpenPrefab(Prefab_ref prefab);
        static void ClosePrefabMode();
    };
}