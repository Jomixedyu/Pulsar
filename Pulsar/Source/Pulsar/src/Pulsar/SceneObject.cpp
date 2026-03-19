#include "SceneObject.h"

namespace pulsar
{

    void SceneObject::SceneObjectConstruct(guid_t guid)
    {
        m_sceneObjectGuid = guid;
        Construct();
    }
}