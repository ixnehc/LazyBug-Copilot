#pragma once

#include <oleidl.h>   // For IOleCommandTarget
#include <unknwn.h>   // For IUnknown
#include <docobj.h>   // For OLECMD, OLECMDF_SUPPORTED, OLECMDF_ENABLED, OLECMDERR_E_NOTSUPPORTED etc.
#include <comdef.h>   // For _com_error (optional, for error handling)
#include <atlbase.h>
#include <atlcom.h>
#include <textmgr.h>  // For IVsTextManager, IVsTextView, IVsTextManagerEvents


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



// 复制完成后，将当前选中引用信息（JSON）以自定义格式追加到剪贴板
void LazyBug_AppendSelectionRefToClipboard();

// 自定义剪贴板格式 ID（LazyBugSelectionRef）
UINT LazyBug_GetSelectionRefClipboardFormat();


// 轻量命令过滤器：只拦截 ECMD_COPY 追加自定义格式，其它命令一律透传
class CCopyRefCommandFilter : public IOleCommandTarget
{
public:
	CCopyRefCommandFilter() : m_cRef(1), m_pNextCmdTarg(nullptr) {}
	virtual ~CCopyRefCommandFilter() {}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
	STDMETHODIMP_(ULONG) AddRef() override;
	STDMETHODIMP_(ULONG) Release() override;

	STDMETHODIMP QueryStatus(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT* pCmdText) override;
	STDMETHODIMP Exec(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT* pvaIn, VARIANT* pvaOut) override;

	void SetNextTarget(IOleCommandTarget* pNextCmdTarg) { m_pNextCmdTarg = pNextCmdTarg; }

private:
	LONG m_cRef;
	IOleCommandTarget* m_pNextCmdTarg;
};


// 监听文本视图创建/销毁，给每个编辑器视图安装 CCopyRefCommandFilter
class CTextViewCreationListener :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IVsTextManagerEvents
{
public:
	CTextViewCreationListener() : m_dwCookie(0) {}

	BEGIN_COM_MAP(CTextViewCreationListener)
		COM_INTERFACE_ENTRY(IVsTextManagerEvents)
	END_COM_MAP()

	HRESULT Advise(IVsTextManager* pTextManager);
	HRESULT Unadvise();

	// IVsTextManagerEvents
	STDMETHODIMP_(void) OnRegisterMarkerType(long iMarkerType) override {}
	STDMETHODIMP_(void) OnRegisterView(IVsTextView* pView) override;
	STDMETHODIMP_(void) OnUnregisterView(IVsTextView* pView) override;
	STDMETHODIMP_(void) OnUserPreferencesChanged(
		const VIEWPREFERENCES* pViewPrefs,
		const FRAMEPREFERENCES* pFramePrefs,
		const LANGPREFERENCES* pLangPrefs,
		const FONTCOLORPREFERENCES* pColorPrefs) override {}

private:
	CComPtr<IVsTextManager> m_pTextManager;
	CComPtr<IConnectionPoint> m_pConnectionPoint;
	DWORD m_dwCookie;
};
