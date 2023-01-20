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
    return Application::Exec(new EditorAppInstance, "apatite", { 1280,720 });
}