#include <PulsarEd/SelectionSet.h>

namespace pulsared
{
    ObjectBase_ref SelectionSet::GetSelected()
    {
        for (int i = this->selection.size() - 1; i >= 0; i--)
        {
            if (IsValid(this->selection[i]))
            {
                return this->selection[i];
            }
        }
        return nullptr;
    }
    array_list<ObjectBase_ref> SelectionSet::GetSelection()
    {
        array_list<ObjectBase_ref> ret;
        for (auto& item : this->selection)
        {
            if (IsValid(item))
            {
                ret.push_back(item);
            }
        }
        return ret;
    }
    void SelectionSet::Select(ObjectBase_ref obj)
    {
        if (!obj) return;
        for (auto& item : this->selection)
        {
            if (item == obj)
            {
                return;
            }
        }

        this->selection.push_back(obj);
    }
    void SelectionSet::UnSelect(ObjectBase_ref obj)
    {
        if (!obj) return;
        for (auto it = this->selection.begin(); it != this->selection.end(); it++)
        {
            if ((*it) == obj)
            {
                it = this->selection.erase(it);
                return;
            }
        }

    }
    bool SelectionSet::IsEmpty()
    {
        if (this->selection.size() == 0)
        {
            return true;
        }
        for (auto it = this->selection.begin(); it != this->selection.end(); it++)
        {
            if (*it)
            {
                return false;
            }
        }

        return true;
    }
    void SelectionSet::Clear()
    {
        this->selection.clear();
    }

    bool SelectionSet::IsType(Type* type)
    {
        if (this->selection.size() == 0)
        {
            return false;
        }

        for (auto& item : this->selection)
        {
            if (item && !type->IsInstanceOfType(item.GetPtr()))
            {
                return false;
            }
        }
        return true;
    }
}