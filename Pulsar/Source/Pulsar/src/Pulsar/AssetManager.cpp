#include <Pulsar/AssetManager.h>
#include <Pulsar/AppInstance.h>
#include <Pulsar/Application.h>


namespace pulsar
{
    AssetManager* AssetManager::Get()
    {
        return Application::inst()->GetAssetManager();
    }

}