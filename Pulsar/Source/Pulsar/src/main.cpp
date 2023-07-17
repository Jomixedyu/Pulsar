#include "Application.h"
#include "EngineAppInstance.h"


int main()
{
    using namespace pulsar;
    Application::Exec(new EngineAppInstance(), "Pulsar", { 1280,720 });
    return 0;
}