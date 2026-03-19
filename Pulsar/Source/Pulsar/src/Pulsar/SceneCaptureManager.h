#pragma once
#include "ObjectBase.h"

namespace pulsar
{
    class SceneCaptureComponent;
    class SceneCaptureManager
    {
    public:

        void Add(SceneCaptureComponent* component)
        {
            m_captures.push_back(component);
        }
        void Remove(const SceneCaptureComponent* component)
        {
            std::erase(m_captures, component);
        }

        const array_list<SceneCaptureComponent*>& GetCaptures() const { return m_captures; }

    protected:
        array_list<SceneCaptureComponent*> m_captures;
    };
}