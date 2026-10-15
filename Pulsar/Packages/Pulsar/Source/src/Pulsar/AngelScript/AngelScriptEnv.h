#pragma once

#include "Scripting/ScriptEnv.h"
#include <angelscript.h>
#include <scriptarray/scriptarray.h>
#include <scriptbuilder/scriptbuilder.h>
#include <scriptdictionary/scriptdictionary.h>
#include <scriptmath/scriptmath.h>
#include <scriptstdstring/scriptstdstring.h>
#include <scripthandle/scripthandle.h>

namespace pulsar
{
    class AngelScriptEnv : public ScriptEnv
    {
    public:
        AngelScriptEnv();
        ~AngelScriptEnv() override;

        bool Initialize();
        void Shutdown();

        asIScriptEngine* GetEngine() const { return m_engine; }

    private:
        static void MessageCallback(const asSMessageInfo* msg, void* param);
        void RegisterAddons();

        asIScriptEngine* m_engine = nullptr;
    };
}
