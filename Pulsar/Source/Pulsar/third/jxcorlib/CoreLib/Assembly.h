#pragma once

#include "Object.h"
#include "Type.h"

namespace jxcorlib
{
    class Assembly;

    class AssemblyManager final
    {
    public:
        static Assembly* BuildAssemblyByName(string_view name);
        static Assembly* FindAssemblyByName(string_view name);
        static void UnloadAssemblyByName(string_view name);
        static Type* GlobalFindType(string_view name);
        static array_list<Assembly*> GetAssemblies();
        static array_list<Type*> GlobalSearchType(Type* baseType);
    };

    class Assembly final : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::Assembly, Object);
        friend class AssemblyManager;
    private:
        string m_name;
        Assembly(string_view name) : m_name(name) {}
    public:
        string GetName() const { return this->m_name; }
    public:
        Type* FindType(string_view name) const;
        array_list<Type*> GetTypes() const
        {
            array_list<Type*> ret;
            for (auto& type : types)
            {
                ret.push_back(type.get());
            }
            return ret;
        }
        void RegisterType(Type* type);

    private:
        array_list<sptr<Type>> types;
    };


}