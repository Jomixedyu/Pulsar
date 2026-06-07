#include "AngelScriptEnv.h"
#include <Pulsar/Logger.h>
#include <format>

namespace pulsar
{
    AngelScriptEnv::AngelScriptEnv() = default;

    AngelScriptEnv::~AngelScriptEnv()
    {
        Shutdown();
    }

    bool AngelScriptEnv::Initialize()
    {
        m_engine = asCreateScriptEngine();
        if (!m_engine)
        {
            Logger::Log("Failed to create AngelScript engine.", LogLevel::Error);
            return false;
        }

        m_engine->SetMessageCallback(asFUNCTION(MessageCallback), this, asCALL_CDECL);
        RegisterAddons();

        return true;
    }

    void AngelScriptEnv::Shutdown()
    {
        if (m_engine)
        {
            m_engine->ShutDownAndRelease();
            m_engine = nullptr;
        }
    }

    void AngelScriptEnv::MessageCallback(const asSMessageInfo* msg, void* param)
    {
        const char* type = "ERR";
        if (msg->type == asMSGTYPE_WARNING)
            type = "WARN";
        else if (msg->type == asMSGTYPE_INFORMATION)
            type = "INFO";

        Logger::Log(std::format("[AngelScript {}] {} ({}:{})", type, msg->message, msg->section, msg->row), LogLevel::Error);
    }

    void AngelScriptEnv::RegisterAddons()
    {
        RegisterStdString(m_engine);
        RegisterScriptArray(m_engine, true);
        RegisterScriptDictionary(m_engine);
        RegisterScriptMath(m_engine);
    }
}
