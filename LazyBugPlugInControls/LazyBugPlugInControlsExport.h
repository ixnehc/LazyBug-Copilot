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

LazyBugPlugInControls_Api void UpdateReload();

LazyBugPlugInControls_Api void AddFileToChat(const unsigned short* fullPath);
