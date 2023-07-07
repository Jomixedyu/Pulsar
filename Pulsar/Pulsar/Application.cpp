#include <Pulsar/Application.h>
#include <Pulsar/Private/SystemInterface.h>
#include <Pulsar/Private/RenderInterface.h>

namespace pulsar
{

    static AppInstance* _current_inst;

    AppInstance* Application::inst()
    {
        return _current_inst;
    }

    int Application::Exec(AppInstance* instance, string_view title, Vector2f size)
    {
        _current_inst = instance;
        instance->OnInitialize(title, size);
        while (!instance->IsQuit())
        {
            instance->OnTick(detail::RenderInterface::GetDeltaTime());
        }
        instance->OnTerminate();
        return 0;
    }

}


