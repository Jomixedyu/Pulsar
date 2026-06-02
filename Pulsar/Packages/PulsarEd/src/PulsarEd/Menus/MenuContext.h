#pragma once
#include <PulsarEd/Assembly.h>

namespace pulsared
{
    class MenuContextBase : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::MenuContextBase, Object);
    public:
    };
    CORELIB_DECL_SHORTSPTR(MenuContextBase);

    class MenuContexts : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::MenuContexts, Object);
    public:
        array_list<MenuContextBase_sp> Contexts;

        string EntryName;

        template<typename T>
        SPtr<T> FindContext()
        {
            Type* type = cltypeof<T>();
            for (auto& context : this->Contexts)
            {
                if (type->IsInstanceOfType(context.get()))
                {
                    return sptr_cast<T>(context);
                }
            }
            return nullptr;
        }


        static SPtr<MenuContexts> StaticMakeContext(string_view entryName, const SPtr<MenuContextBase>& ctx)
        {
            auto ret = mksptr(new MenuContexts);
            ret->EntryName = entryName;
            if (ctx)
                ret->Contexts.push_back(ctx);
            return ret;
        }
    };
    CORELIB_DECL_SHORTSPTR(MenuContexts);
}