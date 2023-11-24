#include <Pulsar/AssetManager.h>
#include <Pulsar/AppInstance.h>
#include <Pulsar/Application.h>


namespace pulsar
{
    AssetManager* GetAssetManager()
    {
        return Application::inst()->GetAssetManager();
    }
}