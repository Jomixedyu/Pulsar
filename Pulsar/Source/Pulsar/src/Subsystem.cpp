#include "Subsystem.h"


namespace pulsar
{




    array_list<Type*>* __PulsarSubsystemRegistry::types()
    {
        static array_list<Type*> types;
        return &types;
    }

}