#pragma once
#include "Assembly.h"
#include <CoreLib/Events.hpp>

namespace apatiteed
{
    class EditorTickerManager
    {
    public:
        static Action<float> EdTicker;
    };

    class IEditorTickable
    {
    public:
        IEditorTickable()
        {
            EditorTickerManager::EdTicker.AddListener(this, &IEditorTickable::OnEditorTick);
        }
        virtual ~IEditorTickable()
        {
            EditorTickerManager::EdTicker.RemoveListener(this, &IEditorTickable::OnEditorTick);
        }

        virtual void OnEditorTick(float dt) = 0;
    };
}