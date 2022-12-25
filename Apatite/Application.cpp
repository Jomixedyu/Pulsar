#include <Apatite/Application.h>
#include <Apatite/Private/SystemInterface.h>
#include <Apatite/Private/RenderInterface.h>

namespace apatite
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


