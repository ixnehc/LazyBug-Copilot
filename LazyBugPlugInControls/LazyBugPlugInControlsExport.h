#pragma once

#ifdef LazyBugPlugInControls_EXPORT
#define LazyBugPlugInControls_Api __declspec(dllexport)
#else
#ifdef LazyBugPlugInControls_IGNORE_IMPORT
#define LazyBugPlugInControls_Api
#else
#define LazyBugPlugInControls_Api __declspec(dllimport)
#endif
#endif


class CTagRichEdit;

struct FileChange;

LazyBugPlugInControls_Api void MfcInit(HINSTANCE hInstance);
LazyBugPlugInControls_Api void MfcTerm();


LazyBugPlugInControls_Api HWND CreateChatDialog(HWND hParent);
LazyBugPlugInControls_Api HWND CreateChangelistsDialog(HWND hParent);
LazyBugPlugInControls_Api void SetFocusToChatInput();

LazyBugPlugInControls_Api BOOL PreTranslateMessageToDialog(HWND hDialog, MSG& msg);

LazyBugPlugInControls_Api void UpdateUI();


LazyBugPlugInControls_Api const unsigned short* GetFileChangeFullPath(const FileChange* change);
LazyBugPlugInControls_Api const unsigned short* FetchFileChangeOpenDocumentRequest();
LazyBugPlugInControls_Api const unsigned short* FetchFileLocatorOpenDocumentRequest(int* outLine = nullptr);
LazyBugPlugInControls_Api bool FetchChatInputEscapeRequest();

//LazyBugPlugInControls_Api bool FetchOpenDocumentRequest(std::wstring& fullFilePath);
LazyBugPlugInControls_Api const FileChange* GetSelectedFileChange();

LazyBugPlugInControls_Api void OpenSolution(const char* slnPath);
LazyBugPlugInControls_Api void CloseSolution();
LazyBugPlugInControls_Api const char* GetOpenedDBFolderPath_utf8();
LazyBugPlugInControls_Api const char* GetOpenedSlnPath_utf8();
LazyBugPlugInControls_Api void EnsureSolutionDBConnected();
LazyBugPlugInControls_Api bool ActivateFileInSolutionDB(const unsigned short* fullPath);

LazyBugPlugInControls_Api void UpdateReload();

LazyBugPlugInControls_Api void AddFileToChat(const unsigned short* fullPath);

// 由 VSIX 侧注册的回调，用于将文件加入当前解决方案中的某个项目。
// 独立运行的 LazyBug.exe 不注册该回调，AddFileToProjectInVS 会返回失败。
typedef bool (*AddFileToProjectFunc)(const unsigned short* projectFilePath, const unsigned short* fileFullPath, char* errorMsg, int errorMsgSize);
LazyBugPlugInControls_Api void SetAddFileToProjectFunc(AddFileToProjectFunc func);

