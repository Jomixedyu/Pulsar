#pragma once
#include <functional>

namespace jxcorelib
{
    template <typename T>
    struct DirtyValue
    {
        explicit DirtyValue(const std::function<T()>& evaluator) : m_evaluator(evaluator)
        {
        }
        void MarkDirty()       { m_dirty = true; }
        bool IsDirty()   const { return m_dirty; }
        T&   Get()
        {
            if (m_dirty)
            {
                m_value = m_evaluator();
                m_dirty = false;
            }
            return m_value;
        }
    private:
        std::function<T()> m_evaluator;
        T    m_value{};
        bool m_dirty = true;
    };
}
