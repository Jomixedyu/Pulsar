#pragma once
#include "ObjectBase.h"

namespace pulsar
{
    class SceneCaptureComponent;
    class SceneCaptureManager
    {
    public:

        void Add(const ObjectPtr<SceneCaptureComponent>& component)
        {
            m_captures.push_back(component);
        }
        void Remove(const ObjectPtr<SceneCaptureComponent>& component)
        {
            std::erase(m_captures, component);
        }

        const array_list<ObjectPtr<SceneCaptureComponent>>& GetCaptures() const { return m_captures; }

    protected:
        array_list<ObjectPtr<SceneCaptureComponent>> m_captures;
    };
}