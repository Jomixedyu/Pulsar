#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include "Defined.h"

#define JXCORLIBPLATFORM_ENABLE_CAPI

namespace jxcorlib::platform::window
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
    intptr_t GetWindowHandlleByPID(intptr_t pid);

    MessageBoxResult MessageBox(intptr_t owner, std::string_view text, std::string_view title, MessageBoxMode mode);

    /**
     * @param filter: u8str, eg: ExeFile(*.exe)|*.exe;TextFile(*.txt)|*.txt
     * @param default_path: u8str
     * @param out_select: u8str
     */
    bool OpenFileDialog(intptr_t owner, std::string_view filter, const std::filesystem::path& default_path, std::filesystem::path* out_select);

    /**
     * Multi-select variant. Returns all selected file paths.
     * @param filter: same format as OpenFileDialog
     */
    bool OpenFileDialogMulti(intptr_t owner, std::string_view filter, const std::filesystem::path& default_path, std::vector<std::filesystem::path>* out_select);

    /**
     * Show a Save File dialog.
     * @param owner: window handle
     * @param filter: same format as OpenFileDialog, eg: PngFile(*.png)|*.png
     * @param default_path: default directory or file path
     * @param out_select: the selected save path
     */
    bool SaveFileDialog(intptr_t owner, std::string_view filter, const std::filesystem::path& default_path, std::filesystem::path* out_select);

    float GetUIScaling();

    intptr_t FindWindow(std::string_view title);
} // namespace jxcorlib::platform::window
