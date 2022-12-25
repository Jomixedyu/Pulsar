#include <Apatite/Logger.h>
#include <iostream>

namespace apatite
{
    using namespace std;

    void Logger::Info(string_view str)
    {
        cout << "[INFO]" << str << endl;
    }
    void Logger::Warning(string_view str)
    {
        cout << "[WARNING]" << str << endl;
    }
    void Logger::Error(string_view str)
    {
        cout << "[ERROR]" << str << endl;
    }
}