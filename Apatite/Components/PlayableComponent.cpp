#include "PlayableComponent.h"

namespace apatite
{
    void PlayableComponent::set_interactive_playing(bool value)
    {
        if (this->enabled_ == value) return;
        this->enabled_ = value;
        if (value)
        {
            this->BeginInteractivePlay();
        }
        else
        {
            this->EndInteractivePlay();
        }
    }
    bool PlayableComponent::get_is_tickable() const
    {
        return this->enabled_;
    }
}