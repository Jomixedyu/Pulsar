#include "Application.h"
#include "EngineAppInstance.h"



int main(int argc, char** argv)
{
    using namespace pulsar;
    Application::Exec(new EngineAppInstance(), "Pulsar", { 1280,720 }, argc, argv);
    return 0;
}
