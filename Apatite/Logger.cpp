#include <Apatite/Logger.h>
#include <iostream>

namespace apatite
{
    using namespace std;
    void Logger::Log(LogLevel level, string_view str)
    {
        switch (level)
        {
        case apatite::LogLevel::Info:
            cout << "[INFO]";
            break;
        case apatite::LogLevel::Warning:
            cout << "[WARNING]" ;
            break;
        case apatite::LogLevel::Error:
            cout << "[ERROR]";
            break;
        default:
            cout << "[Unknown]";
            break;
        }
        cout << str << endl;
        LogListener.Invoke(level, str);
    }

}