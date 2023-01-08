#include <ApatiteEd/SelectionSet.h>

namespace apatiteed
{
    ObjectBase_sp SelectionSet::GetSelected()
    {
        for (int i = this->selection.size() - 1; i >= 0; i--)
        {
            if (!this->selection[i].expired())
            {
                return this->selection[i].lock();
            }
        }
        return nullptr;
    }
    array_list<ObjectBase_sp> SelectionSet::GetSelection()
    {
        array_list<ObjectBase_sp> ret;
        for (auto& item : this->selection)
        {
            if (!item.expired())
            {
                ret.push_back(item.lock());
            }
        }
        return ret;
    }
    void SelectionSet::Select(ObjectBase_rsp obj)
    {
        if (obj == nullptr) return;
        for (auto& item : this->selection)
        {
            if (!item.expired() && item.lock() == obj)
            {
                return;
            }
        }

        this->selection.push_back(ObjectBase_wp{ obj });
    }
    void SelectionSet::UnSelect(ObjectBase_rsp obj)
    {
        if (obj == nullptr) return;
        for (auto it = this->selection.begin(); it != this->selection.end(); it++)
        {
            if (!it->expired() && it->lock() == obj)
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
            if (!it->expired())
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
            if (!item.expired() && !type->IsInstanceOfType(item.lock().get()))
            {
                return false;
            }
        }
        return true;
    }
}