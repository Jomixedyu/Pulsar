#pragma once
#include "Assembly.h"
#include <CoreLib/Events.hpp>

namespace pulsared
{
    template <typename T>
    class SelectionSet
    {
    public:
        ObjectPtr<T> GetSelected() const
        {
            for (int i = (int)this->selection.size() - 1; i >= 0; i--)
            {
                if (this->selection[i])
                {
                    return this->selection[i];
                }
            }
            return nullptr;
        }
        array_list<ObjectPtr<T>> GetSelection() const
        {
            array_list<ObjectPtr<T>> ret;
            for (auto& item : this->selection)
            {
                if (item)
                {
                    ret.push_back(item);
                }
            }
            return ret;
        }
        void Select(ObjectPtr<T> obj)
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
        void UnSelect(ObjectPtr<T> obj)
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
        bool IsEmpty() const
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
        void Clear()
        {
            this->selection.clear();
        }
        bool IsType(Type* type) const
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

    protected:
        array_list<ObjectPtr<T>> selection;
    };

} // namespace pulsared