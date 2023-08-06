#include "Assembly.h"
#include <cassert>

namespace jxcorlib
{

    Type* Assembly::FindType(string_view name) const
    {
        for (auto item : this->types)
        {
            if (item->GetName() == name)
            {
                return item;
            }
        }
        return nullptr;
    }

    void Assembly::RegisterType(Type* type)
    {
        this->types.push_back(type);
    }



    static array_list<Assembly*>& assemblies()
    {
        static array_list<Assembly*> list;
        return list;
    }
    

    Assembly* AssemblyManager::FindAssemblyByName(string_view name)
    {
        for (Assembly* assembly : assemblies())
        {
            if (assembly->GetName() == name)
            {
                return assembly;
            }
        }
        return nullptr;
    }

    void AssemblyManager::UnloadAssemblyByName(string_view name)
    {
        for (auto it = assemblies().begin(); it < assemblies().end(); ++it)
        {
            if ((*it)->GetName() == name)
            {
                assemblies().erase(it);
                return;
            }
        }
    }
    Assembly* AssemblyManager::BuildAssemblyByName(string_view name)
    {
        Assembly* ass = FindAssemblyByName(name);

        if (ass == nullptr)
        {
            ass = new Assembly(name);
            assemblies().push_back(ass);
        }
        return ass;
    }

}

