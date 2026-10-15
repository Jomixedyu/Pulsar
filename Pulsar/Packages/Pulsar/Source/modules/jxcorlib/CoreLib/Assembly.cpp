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
                return item.get();
            }
        }
        return nullptr;
    }

    void Assembly::RegisterType(Type* type)
    {
        this->types.push_back(mksptr(type));
    }



    static array_list<Assembly*>& assemblies()
    {
        static array_list<Assembly*> list;
        return list;
    }

    array_list<Assembly*> AssemblyManager::GetAssemblies()
    {
        return assemblies();
    }

    Assembly* AssemblyManager::FindAssemblyByName(string_view name)
    {
        for (Assembly* assembly : assemblies())
        {
            auto asmName = assembly->GetName();
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
    Type* AssemblyManager::GlobalFindType(string_view name)
    {
        auto index = name.find(':');
        if (auto assembly = FindAssemblyByName(name.substr(0, index)))
        {
            return assembly->FindType(name);
        }
        return nullptr;
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
    array_list<Type*> AssemblyManager::GlobalSearchType(Type* baseType)
    {
        array_list<Type*> ret;
        for (auto& assembly : assemblies())
        {
            for (auto& type : assembly->types)
            {
                if (type->IsSubclassOf(baseType))
                {
                    ret.push_back(type.get());
                }
            }
            
        }
        return ret;
    }
}

