#include "Assembly.h"

namespace jxcorlib
{

    Type* Assembly::FindType(string_view name) const
    {
        for (auto item : this->types)
        {
            if (item->get_name() == name)
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

    Assembly* Assembly::StaticFindAssemblyByName(string_view name)
    {
        for (Assembly* assembly : assemblies())
        {
            if (assembly->get_name() == name)
            {
                return assembly;
            }
        }
        return nullptr;
    }

    void Assembly::StaticUnloadAssemblyByName(string_view name)
    {

    }
    Assembly* Assembly::StaticBuildAssemblyByName(string_view name)
    {
        Assembly* ass = StaticFindAssemblyByName(name);
        if (ass == nullptr)
        {
            ass = new Assembly(name);
            assemblies().push_back(ass);
        }
        return ass;
    }

}

