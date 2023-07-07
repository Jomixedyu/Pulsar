#include <Pulsar/Application.h>
#include <Pulsar/EngineAppInstance.h>
#include <PulsarEd/EditorAppInstance.h>
#include <PulsarEd/EditorUIConfig.h>
#include <CoreLib/File.h>
#include <Windows.h>
#include <iostream>

using namespace std;
using namespace pulsar;
using namespace pulsared;

#include <Pulsar/TransformUtil.h>

int main()
{
    return Application::Exec(new EditorAppInstance, "pulsar", { 1280,720 });
}