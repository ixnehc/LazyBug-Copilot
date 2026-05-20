#pragma once

#include <oleidl.h>   // For IOleCommandTarget
#include <unknwn.h>   // For IUnknown
#include <docobj.h>   // For OLECMD, OLECMDF_SUPPORTED, OLECMDF_ENABLED, OLECMDERR_E_NOTSUPPORTED etc.
#include <comdef.h>   // For _com_error (optional, for error handling)

// 你需要包含 Visual Studio SDK 的头文件来获取命令组 GUID 和命令 ID
// 例如：vsshlids.h (通常包含了常用的命令定义)
// #include <vsshlids.h>


// 示例 VSStd2KCmdID (确保从 vsshlids.h 或相关文档中获取准确值)
enum VSStd2KCmdID_Example 
{
	ECMD_TYPECHAR_EXAMPLE = 104, // 实际值可能不同
	ECMD_PASTE_EXAMPLE = 57,
	ECMD_DELETE_EXAMPLE = 17,
	ECMD_BACKSPACE_EXAMPLE = 3,
	// ... 其他你关心的命令
};


class CCommandFilter : public IOleCommandTarget
{
public:
	CCommandFilter();
	virtual ~CCommandFilter();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
	STDMETHODIMP_(ULONG) AddRef() override;
	STDMETHODIMP_(ULONG) Release() override;

	// IOleCommandTarget methods
	STDMETHODIMP QueryStatus(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT* pCmdText) override;
	STDMETHODIMP Exec(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT* pvaIn, VARIANT* pvaOut) override;

	// 设置命令链中的下一个目标
	void SetNextTarget(IOleCommandTarget* pNextCmdTarg);

private:
	LONG m_cRef;
	IOleCommandTarget* m_pNextCmdTarg; // 命令链中的下一个 IOleCommandTarget
};