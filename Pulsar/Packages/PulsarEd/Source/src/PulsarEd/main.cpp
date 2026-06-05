#include <CoreLib/File.h>
#include <Pulsar/Application.h>
#include <Pulsar/EngineAppInstance.h>
#include <PulsarEd/EditorAppInstance.h>
#include <PulsarEd/EditorUIConfig.h>
#include <Windows.h>
#include <iostream>

using namespace std;
using namespace pulsar;
using namespace pulsared;

std::wstring String2Wstring(std::string wstr)
{
    std::wstring res;
    int len = MultiByteToWideChar(CP_ACP, 0, wstr.c_str(), wstr.size(), nullptr, 0);
    if( len < 0 ){
        return res;
    }
    wchar_t* buffer = new wchar_t[len + 1];
    if( buffer == nullptr){
        return res;
    }
    MultiByteToWideChar(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len);
    buffer[len] = '\0';
    res.append(buffer);
    delete[] buffer;
    return res;
}

int main(int argc, char** argv)
{
    int code = 0;
//    try
//    {
        code = Application::Exec(new EditorAppInstance, "Pulsar Editor", { 1280,720 }, argc, argv);
//    }
//    catch(std::exception& e)
//    {
//
//        _wassert(String2Wstring(e.what()).c_str(), _CRT_WIDE(__FILE__), __LINE__);
//
//    }
    return code;
}