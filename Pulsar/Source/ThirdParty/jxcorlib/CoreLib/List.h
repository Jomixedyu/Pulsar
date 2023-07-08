#pragma once
#include "Object.h"
#include "Type.h"
#include "Assembly.h"
#include "BasicTypes.h"

namespace jxcorlib
{
    class ArrayList : public Object, public array_list<Object_sp>, public IList
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::ArrayList, Object);
        CORELIB_IMPL_INTERFACES(IList);
    public:
        ArrayList() : CORELIB_INIT_INTERFACE(IList) {}
        ArrayList(std::initializer_list<Object_sp> list) : CORELIB_INIT_INTERFACE(IList)
        {
            for (auto& item : list) this->push_back(item);
        }
        virtual void Add(Object_rsp value) override
        {
            this->push_back(value);
        }
        virtual Object_sp At(int32_t index) const override
        {
            return this->at(index);
        }
        virtual void Clear() override { this->clear(); }
        virtual void RemoveAt(int32_t index) override
        {
            this->erase(this->begin() + index);
        }
        virtual int32_t IndexOf(Object_rsp value) const override;

        virtual bool Contains(Object_rsp value) override
        {
            return this->IndexOf(value) >= 0;
        }
        virtual int32_t GetCount() const override { return static_cast<int32_t>(this->size()); }

        virtual bool Equals(Object* obj) const override;

        virtual Type* GetIListElementType() const override { return cltypeof<Object>(); }
    private:

    };
    CORELIB_DECL_SHORTSPTR(ArrayList);


    template<typename T>
    class List : public Object, public array_list<T>, public IList
    {
        CORELIB_DEF_TEMPLATE_TYPE(AssemblyObject_jxcorlib, jxcorlib::List, Object, T);
        CORELIB_IMPL_INTERFACES(IList);
        static_assert((cltype_concept<T>&& is_shared_ptr<T>::value) || !cltype_concept<T>, "");
        constexpr static bool is_shared_cltype = cltype_concept<T> && is_shared_ptr<T>::value;
    public:
        List() : CORELIB_INIT_INTERFACE(IList) {}
        List(std::initializer_list<T> list) : CORELIB_INIT_INTERFACE(IList)
        {
            for (auto& item : list) this->push_back(item);
        }
        virtual void Add(Object_rsp value) override
        {
            if constexpr (is_shared_cltype)
            {
                this->push_back(std::static_pointer_cast<typename remove_shared_ptr<T>::type>(value));
            }
            else
            {
                this->push_back(UnboxUtil::Unbox<T>(value));
            }
        }
        virtual Object_sp At(int32_t index) const override
        {
            if constexpr (is_shared_cltype)
            {
                return std::static_pointer_cast<Object>(this->at(index));
            }
            else
            {
                return BoxUtil::Box<T>(this->at(index));
            }
        }
        virtual void Clear() override { this->clear(); }
        virtual void RemoveAt(int32_t index) override
        {
            this->erase(this->begin() + index);
        }
        virtual int32_t IndexOf(Object_rsp value) const override
        {
            for (int32_t i = 0; i < this->size(); i++)
            {
                auto& item = this->at(i);
                if constexpr (is_shared_cltype)
                {
                    if (value == nullptr && item == nullptr)
                    {
                        return i;
                    }
                    else if (item != nullptr && item->Equals(value.get()))
                    {
                        return i;
                    }
                }
                else
                {
                    if (item == UnboxUtil::Unbox<T>(value))
                    {
                        return i;
                    }
                }
            }
            return -1;
        }
        virtual bool Contains(Object_rsp value) override
        {
            return this->IndexOf(value) >= 0;
        }
        virtual int32_t GetCount() const override { return static_cast<int32_t>(this->size()); }

        virtual Type* GetIListElementType() const override { return cltypeof<typename get_boxing_type<T>::type>(); }

        virtual bool Equals(Object* obj) const override
        {
            if (obj == nullptr) return false;
            if (this->GetType() != obj->GetType()) return false;

            List<T>* list = static_cast<List<T>*>(obj);
            intptr_t t = (intptr_t)obj;
            intptr_t t2 = (intptr_t)list;

            if (this->size() != list->size()) return false;

            if constexpr (is_shared_cltype)
            {
                for (int i = 0; i < this->size(); i++)
                {
                    if (!this->At(i)->Equals(list->At(i).get()))
                    {
                        return false;
                    }
                }
            }
            else
            {
                for (int i = 0; i < this->size(); i++)
                {
                    if (this->at(i) != list->at(i))
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    };
    CORELIB_DECL_TEMP_SHORTSPTR(List);

#define CORELIB_DECL_LIST(T) \
namespace jxcorlib \
{ \
    template class List<T>; \
}

}
