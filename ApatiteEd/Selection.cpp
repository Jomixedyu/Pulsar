#include <ApatiteEd/Selection.h>

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
            ret.push_back(item.lock());
        }
        return ret;
    }
    void SelectionSet::Select(ObjectBase_rsp obj)
    {
        for (auto& item : this->selection)
        {
            if (item.lock() == obj)
            {
                return;
            }
        }

        this->selection.push_back(ObjectBase_wp{ obj });
    }
    void SelectionSet::UnSelect(ObjectBase_rsp obj)
    {
        for (auto it = this->selection.begin(); it != this->selection.end(); it++)
        {
            if (it->lock() == obj)
            {
                it = this->selection.erase(it);
                return;
            }
        }

    }
}