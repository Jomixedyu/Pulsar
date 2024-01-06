#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace jxcorlib::platform
{
    namespace window
    {
        enum class MessageBoxMode : uint8_t
        {
            YesNo,
            Ok,
            OkCancel,
            YesNoCancel
        };
        enum class MessageBoxResult : uint8_t
        {
            Ok,
            Cancel,
            Yes,
            No,
        };

        intptr_t GetMainWindowHandle();

        MessageBoxResult MessageBox(intptr_t owner, std::string_view text, std::string_view title, MessageBoxMode mode);
        
        /**
        * @param filter: u8str, eg: ExeFile(*.exe)|*.exe;TextFile(*.txt)|*.txt
        * @param default_path: u8str
        * @param out_select: u8str
        */
        bool OpenFileDialog(intptr_t owner, std::string_view filter, std::filesystem::path default_path, std::filesystem::path* out_select);

        float GetUIScaling();
    }
}