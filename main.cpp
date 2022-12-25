#include <Apatite/Application.h>
#include <Apatite/EngineAppInstance.h>

using namespace apatite;
int main()
{
    return Application::Exec(new EngineAppInstance, "apatite", { 1280,720 });
}