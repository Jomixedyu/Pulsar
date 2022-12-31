#include "Subsystem.h"


namespace apatite
{




    array_list<Type*>* __ApatiteSubsystemRegistry::types()
    {
        array_list<Type*> types;
        return &types;
    }

}