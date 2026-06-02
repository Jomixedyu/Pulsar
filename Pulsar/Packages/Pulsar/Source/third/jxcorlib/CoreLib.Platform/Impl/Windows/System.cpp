#include "System.h"
#include <Windows.h>

namespace jxcorlib::platform::system
{
    void OpenFileBrowser(const std::filesystem::path& path)
    {
        ShellExecute(NULL, "open", path.string().c_str(), NULL, NULL, SW_SHOW);
    }
} // namespace jxcorlib::platform::system
