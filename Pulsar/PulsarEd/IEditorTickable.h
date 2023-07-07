#pragma once
#include "Assembly.h"
#include <CoreLib/Events.hpp>

namespace pulsared
{
    class EditorTickerManager
    {
    public:
        static inline Action<float> Ticker;
    };

    class IEditorTickable
    {
    public:
        IEditorTickable()
        {
            EditorTickerManager::Ticker.AddListener(this, &IEditorTickable::OnEditorTick);
        }
        virtual ~IEditorTickable()
        {
            EditorTickerManager::Ticker.RemoveListener(this, &IEditorTickable::OnEditorTick);
        }

        virtual void OnEditorTick(float dt) = 0;
    };
}