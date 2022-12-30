#pragma once
#include <ApatiteEd/Assembly.h>

namespace apatiteed
{
    class MenuContextBase : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatiteed::MenuContextBase, Object);
    public:
    };

    class MenuContexts : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatiteed::MenuContexts, Object);
    public:
        array_list<sptr<MenuContextBase>> contexts;

        string entry_name;

        template<typename T>
        sptr<T> FindContext()
        {
            Type* type = cltypeof<T>();
            for (auto& context : this->contexts)
            {
                if (type->IsInstanceOfType(context.get()))
                {
                    return sptr_cast<T>(context);
                }
            }
            return nullptr;
        }


        static sptr<MenuContexts> StaticMakeContext(string_view entry_name, const sptr<MenuContextBase>& ctx)
        {
            auto ret = mksptr(new MenuContexts);
            ret->entry_name = entry_name;
            if (ctx)
                ret->contexts.push_back(ctx);
            return ret;
        }
    };
    CORELIB_DECL_SHORTSPTR(MenuContexts);
}