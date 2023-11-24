#include "List.h"


namespace jxcorlib
{

    int32_t ArrayList::IndexOf(Object_rsp value) const
    {
        for (int32_t i = 0; i < this->size(); i++)
        {
            auto& item = this->at(i);
            if (value == nullptr && item == nullptr)
            {
                return i;
            }
            else if (item != nullptr && item->Equals(value.get()))
            {
                return i;
            }
        }
        return -1;
    }

    bool ArrayList::Equals(Object* obj) const
    {
        if (obj == nullptr) return false;
        if (this->GetType() != obj->GetType()) return false;

        auto list = static_cast<ArrayList*>(obj);
        intptr_t t = (intptr_t)obj;
        intptr_t t2 = (intptr_t)list;


        int lista = this->size();
        int listb = list->size();
        if (this->size() != list->size()) return false;


        for (int i = 0; i < this->size(); i++)
        {
            if (!this->At(i)->Equals(list->At(i).get()))
            {
                return false;
            }
        }

        return true;
    }

}