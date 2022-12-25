#include <Apatite/Application.h>
#include <Apatite/EngineAppInstance.h>
#include <ApatiteEd/EditorAppInstance.h>

using namespace apatite;
using namespace apatiteed;
int main()
{
    return Application::Exec(new EditorAppInstance, "apatite", { 1280,720 });
}