#include "../../Window.h"

#include <Windows.h>
#include <commdlg.h>
#include <vector>
#include <filesystem>

namespace jxcorlib::platform::window
{
    using std::string;

    static UINT _MappingMsgMode(MessageBoxMode mode)
    {
        switch (mode)
        {
        case jxcorlib::platform::window::MessageBoxMode::YesNo: return MB_YESNO;
        case jxcorlib::platform::window::MessageBoxMode::Ok: return MB_OK;
        case jxcorlib::platform::window::MessageBoxMode::OkCancel: return MB_OKCANCEL;
        case jxcorlib::platform::window::MessageBoxMode::YesNoCancel: return MB_YESNOCANCEL;
        default:
            break;
        }
        return MB_OK;
    }
    static string UTF8ToANSI(const char* src_str)
    {
        int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
        wchar_t* wszANSI = new wchar_t[len + 1];
        memset(wszANSI, 0, len * 2 + 2);
        MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszANSI, len);
        len = WideCharToMultiByte(CP_ACP, 0, wszANSI, -1, NULL, 0, NULL, NULL);
        char* szANSI = new char[len + 1];
        memset(szANSI, 0, len + 1);
        WideCharToMultiByte(CP_ACP, 0, wszANSI, -1, szANSI, len, NULL, NULL);
        string strTemp(szANSI);
        if (wszANSI) delete[] wszANSI;
        if (szANSI) delete[] szANSI;
        return strTemp;
    }

    static string ANSIToUTF8(const char* src_str)
    {
        string outUtf8 = "";
        int n = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
        WCHAR* str1 = new WCHAR[n];
        MultiByteToWideChar(CP_ACP, 0, src_str, -1, str1, n);
        n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
        char* str2 = new char[n];
        WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
        outUtf8 = str2;
        delete[]str1;
        str1 = NULL;
        delete[]str2;
        str2 = NULL;
        return outUtf8;
    }

    static MessageBoxResult _MappingMsgResult(int ret)
    {
        switch (ret)
        {
        case IDCANCEL: return MessageBoxResult::Cancel;
        case IDNO: return MessageBoxResult::No;
        case IDOK: return MessageBoxResult::Ok;
        case IDYES: return MessageBoxResult::Yes;
        default:
            break;
        }
        return MessageBoxResult::Cancel;
    }

    static MessageBoxResult _MessageBoxImpl(intptr_t owner, std::string_view text, std::string_view title, MessageBoxMode mode)
    {
        auto ret = MessageBox((HWND)owner, UTF8ToANSI(text.data()).data(), UTF8ToANSI(title.data()).data(), _MappingMsgMode(mode));
        return _MappingMsgResult(ret);
    }

#undef MessageBox

    typedef struct tagWNDINFO
    {
        DWORD dwProcessId;
        HWND hWnd;
    } WNDINFO, * LPWNDINFO;

    static BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam)
    {
        DWORD dwProcessId;
        GetWindowThreadProcessId(hWnd, &dwProcessId);
        LPWNDINFO pInfo = (LPWNDINFO)lParam;

        if (dwProcessId == pInfo->dwProcessId)
        {
            pInfo->hWnd = hWnd;
            return FALSE;
        }
        return TRUE;
    }

    intptr_t GetMainWindowHandle()
    {
        return GetWindowHandlleByPID(GetCurrentProcessId());
    }

    intptr_t GetWindowHandlleByPID(intptr_t pid)
    {
        WNDINFO wi;
        wi.dwProcessId = pid;
        wi.hWnd = nullptr;
        EnumWindows(EnumProc, (LPARAM)&wi);
        return (intptr_t)wi.hWnd;
    }

    MessageBoxResult MessageBox(intptr_t owner, std::string_view text, std::string_view title, MessageBoxMode mode)
    {
        return _MessageBoxImpl(owner, text, title, mode);
    }

    // Helper: build the null-delimited filter string used by OPENFILENAME
    static char* BuildFilterStr(std::string_view _filter)
    {
        auto filter = UTF8ToANSI(_filter.data());
        char* filter_str = new char[filter.length() + 2];
        for (size_t i = 0; i < filter.length(); i++)
        {
            char c = filter[i];
            if (c == ';')       filter_str[i] = '\0';
            else if (c == '|')  filter_str[i] = ';';
            else                filter_str[i] = c;
        }
        filter_str[filter.length()]     = 0;
        filter_str[filter.length() + 1] = 0;
        return filter_str;
    }

    bool OpenFileDialog(intptr_t owner, std::string_view _filter, const std::filesystem::path& default_path, std::filesystem::path* out_select)
    {
        OPENFILENAME ofn = { 0 };
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = (HWND)owner;

        char* filter_str = BuildFilterStr(_filter);
        auto default_dir = UTF8ToANSI(default_path.string().c_str());

        TCHAR szBuffer[MAX_PATH] = { 0 };
        ofn.lpstrFilter    = filter_str;
        ofn.lpstrInitialDir = default_dir.c_str();
        ofn.lpstrFile      = szBuffer;
        ofn.nMaxFile       = sizeof(szBuffer) / sizeof(*szBuffer);
        ofn.nFilterIndex   = 0;
        ofn.Flags          = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;

        BOOL bSel = GetOpenFileName(&ofn);
        delete[] filter_str;

        if (bSel)
        {
            if (out_select) *out_select = szBuffer;
            return true;
        }
        return false;
    }

    bool OpenFileDialogMulti(intptr_t owner, std::string_view _filter, const std::filesystem::path& default_path, std::vector<std::filesystem::path>* out_select)
    {
        OPENFILENAME ofn = { 0 };
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = (HWND)owner;

        char* filter_str = BuildFilterStr(_filter);
        auto default_dir = UTF8ToANSI(default_path.string().c_str());

        // Large buffer: directory + '\0' + file1 '\0' file2 '\0' ... '\0' '\0'
        const DWORD bufSize = 32768;
        std::vector<TCHAR> szBuffer(bufSize, 0);
        ofn.lpstrFilter     = filter_str;
        ofn.lpstrInitialDir = default_dir.c_str();
        ofn.lpstrFile       = szBuffer.data();
        ofn.nMaxFile        = bufSize;
        ofn.nFilterIndex    = 0;
        ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER
                            | OFN_NOCHANGEDIR   | OFN_ALLOWMULTISELECT;

        BOOL bSel = GetOpenFileName(&ofn);
        delete[] filter_str;

        if (!bSel) return false;

        if (out_select)
        {
            // When multiple files are selected the buffer contains:
            //   "dir\0file1\0file2\0...\0\0"
            // When only one file: "full_path\0\0"
            const TCHAR* p = szBuffer.data();
            std::filesystem::path dir(p);
            p += dir.native().size() + 1;

            if (*p == 0)
            {
                // Single selection – p is empty, dir is the full path
                out_select->push_back(dir);
            }
            else
            {
                while (*p != 0)
                {
                    out_select->push_back(dir / p);
                    p += std::char_traits<TCHAR>::length(p) + 1;
                }
            }
        }
        return true;
    }

    float GetUIScaling()
    {
        const HDC hDC = ::GetDC(nullptr);
        const UINT dpix = ::GetDeviceCaps(hDC, LOGPIXELSX);
        ::ReleaseDC(nullptr, hDC);
        return (float)dpix / 96.f;
    }
}