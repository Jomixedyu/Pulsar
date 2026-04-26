#include "InputSystem/InputActionMap.h"

#include <ranges>

namespace pulsar
{
    void InputActionMap::OnConstruct()
    {
        base::OnConstruct();
        init_sptr_member(m_actionNames);
    }

    int InputActionMap::NewAction(string_view name)
    {
        string n{name};

        if (std::ranges::contains(*m_actionNames, n))
        {
            return -1;
        }

        auto newPair = mksptr(new ActionBindingPair);

        m_actions.push_back(newPair);
        m_actionNames->push_back(n);

        return (int)m_actionNames->size() - 1;
    }

    void InputActionMap::RemoveAction(string_view name)
    {
        int index = 0;
        bool found = false;
        for (auto& value : *m_actionNames)
        {
            if (value == name)
            {
                found = true;
                break;
            }
            ++index;
        }
        if (found)
        {
            RemoveActionAt(index);
        }

    }

    void InputActionMap::RemoveActionAt(int index)
    {
        if (IsValidIndex(index))
        {
            m_actionNames->erase(m_actionNames->begin() + index);
            m_actions.erase(m_actions.begin() + index);
        }
    }

    bool InputActionMap::IsValidIndex(int index) const
    {
        return index > -1 && index < m_actionNames->size();
    }

} // namespace pulsar
