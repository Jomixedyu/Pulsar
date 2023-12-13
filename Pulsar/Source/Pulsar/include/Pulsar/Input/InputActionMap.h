#pragma once
#include "InputAction.h"
#include "Pulsar/AssetObject.h"

namespace pulsar
{

    class InputActionMap : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputActionMap, AssetObject);
    public:
        void AddInputAction(InputSlot);
    protected:

    };
} // namespace pulsar
