#include "Subsystem.h"


namespace apatite
{




    array_list<Type*>* __ApatiteSubsystemRegistry::types()
    {
        static array_list<Type*> types;
        return &types;
    }

}