#include "GlobalSettings.h"

namespace pulsar
{
    GlobalSettings::GlobalSettings()
        : m_renderSettings(mksptr(new RenderSettings()))
    {
    }

    Object* GlobalSettings::Register(Type* type)
    {
        if (!type)
            return nullptr;

        auto it = m_registeredSettings.find(type);
        if (it != m_registeredSettings.end())
            return it->second.get();

        auto settings = type->CreateSharedInstance({});
        auto* value = settings.get();
        m_registeredSettings.emplace(type, std::move(settings));
        return value;
    }

    Object* GlobalSettings::Get(Type* type)
    {
        auto it = m_registeredSettings.find(type);
        return it == m_registeredSettings.end() ? nullptr : it->second.get();
    }
}