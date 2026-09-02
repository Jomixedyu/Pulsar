#include <CoreLib.Platform/FolderWatch.h>
#include <Windows.h>

#define GET_HANDLE() ((_FolderWatchData*)this->handle_)

namespace jxcorlib::platform
{
    struct _FolderWatchData
    {
        HANDLE file_handle;
        HANDLE event;
    };

    FolderWatch::FolderWatch(std::string_view path, bool recursive)
    {
        this->path_ = path;
        this->recursive_ = recursive;
        this->handle_ = new _FolderWatchData;

        GET_HANDLE()->event = CreateEvent(NULL, FALSE, FALSE, NULL);
        GET_HANDLE()->file_handle = CreateFile(
            this->path_.c_str(),                                // pointer to the file name
            FILE_LIST_DIRECTORY,                // access (read/write) mode
            // Share mode MUST be the following to avoid problems with renames via Explorer!
            FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, // share mode
            NULL,                               // security descriptor
            OPEN_EXISTING,                      // how to create
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,         // file attributes
            NULL                                // file with attributes to copy
        );    
        if (GET_HANDLE()->file_handle == INVALID_HANDLE_VALUE) //若网络重定向或目标文件系统不支持该操作，函数失败，同时调用GetLastError()返回ERROR_INVALID_FUNCTION
        {
            throw GetLastError();
        }

    }
    FolderWatch::~FolderWatch()
    {
        CloseHandle(GET_HANDLE()->file_handle);
        delete this->handle_;
    }

    void FolderWatch::Tick()
    {
        TCHAR szBuffer[65535];
        DWORD BytesReturned;

        OVERLAPPED stOverlapped;
        ZeroMemory(&stOverlapped, sizeof(stOverlapped));

        stOverlapped.hEvent = GET_HANDLE()->event;

        ReadDirectoryChangesW(
            ((_FolderWatchData*)this->handle_)->file_handle,                          // handle to directory
            &szBuffer,                     // read results buffer
            sizeof(szBuffer),              // length of buffer
            (this->recursive_ ? TRUE : FALSE),    // monitoring option
            FILE_NOTIFY_CHANGE_SECURITY |
            FILE_NOTIFY_CHANGE_CREATION |
            FILE_NOTIFY_CHANGE_LAST_WRITE |
            FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_ATTRIBUTES |
            FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_FILE_NAME,  // filter conditions
            &BytesReturned,                // bytes returned
            &stOverlapped,                 // overlapped buffer
            NULL                           // completion routine
        );

        //DWORD dwWaitRes = WaitForMultipleObjects(2, events, FALSE, INFINITE);

        ////terminating event was raised
        //if (dwWaitRes == WAIT_OBJECT_0 + 1) {
        //    return;
        //}

        DWORD dwBytesRead = 0;
        PFILE_NOTIFY_INFORMATION pInfo = NULL;
        GetOverlappedResult(GET_HANDLE()->file_handle, &stOverlapped, &dwBytesRead, FALSE);

        pInfo = (PFILE_NOTIFY_INFORMATION)(&szBuffer);

        DWORD fileNameLength = pInfo->FileNameLength / sizeof(WCHAR);

        char* szFileName = new char[fileNameLength + 1];
        memset(szFileName, 0, fileNameLength + 1);

        WideCharToMultiByte(CP_OEMCP, NULL, pInfo->FileName, pInfo->FileNameLength / sizeof(WCHAR), szFileName, fileNameLength / sizeof(TCHAR), NULL, NULL);

        FileActionType fat;
        switch (pInfo->Action) {
        case FILE_ACTION_ADDED:            fat = FileActionType::Add;       break;
        case FILE_ACTION_REMOVED:          fat = FileActionType::Remove;    break;
        case FILE_ACTION_MODIFIED:         fat = FileActionType::Modify;    break;
        case FILE_ACTION_RENAMED_OLD_NAME: fat = FileActionType::RenameOld; break;
        case FILE_ACTION_RENAMED_NEW_NAME: fat = FileActionType::RenameNew; break;
        }

        delete[] szFileName;
    }
}