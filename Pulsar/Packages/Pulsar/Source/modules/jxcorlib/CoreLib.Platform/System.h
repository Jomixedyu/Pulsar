#pragma once
#include <string_view>
#include <string>
#include <filesystem>

namespace jxcorlib::platform::system
{
    void OpenFileBrowser(const std::filesystem::path& path);
}