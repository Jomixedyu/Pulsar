#pragma once

#include "ObjectBase.h"

namespace apatite
{
    class Logger
    {
    public:
        static void Info(string_view str);
        static void Warning(string_view str);
        static void Error(string_view str);
    };

}
