#include <Apatite/Application.h>
#include <Apatite/EngineAppInstance.h>
#include <ApatiteEd/EditorAppInstance.h>
#include <ApatiteEd/EditorUIConfig.h>
#include <CoreLib/File.h>
#include <Windows.h>
#include <iostream>

using namespace std;
using namespace apatite;
using namespace apatiteed;

#include <Apatite/TransformUtil.h>

int main()
{

    //auto q4 = Quat4f::FromEuler({ 50,90,45 });
    //cout << to_string(q4) << endl;
    //auto e = q4.GetEuler();
    //cout << to_string(e) << endl;
    //return 0;
    return Application::Exec(new EditorAppInstance, "apatite", { 1280,720 });
}