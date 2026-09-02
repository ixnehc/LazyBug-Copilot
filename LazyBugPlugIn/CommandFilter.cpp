#include "stdh.h"
#include "CommandFilter.h"
#include <string>    // For std::wstring (用于调试输出)
#include <vector>    // If managing multiple commands

#include "PackageState.h"
#include "..\LazyBugPlugInUI\CommandIds.h"
#include "FileChangeAttach.h"
#include "Utils.h"

//#include "stdidcmd.h"
//ECMD_TYPECHAR

GUID CMDSETID_Standard2K = { 0x1496a755, 0x94de, 0x11d0, {0x8c, 0x3f, 0x0, 0xc0, 0x4f, 0xc2, 0xaa, 0xe2} };

const GUID CGID_VSStandardCommandSet97 = {0x5EFC7975, 0x14BC, 0x11CF, {0x9B, 0x2B, 0x00, 0xAA, 0x00, 0x57, 0x38, 0x19}};

// LazyBug 自定义命令集 GUID（与 guids.h 中 guidLazyBugPlugInCmdSet 一致）
const GUID guidLazyBugCmdSet = { 0xCFCA783F, 0x6B83, 0x445D, { 0xA4, 0x7, 0x9, 0xF8, 0xCE, 0xCB, 0x57, 0xE4 } };


// 构造函数
CCommandFilter::CCommandFilter() : m_cRef(1), m_pNextCmdTarg(nullptr)
{
}

// 析构函数
CCommandFilter::~CCommandFilter()
{
	if (m_pNextCmdTarg)
	{
		m_pNextCmdTarg->Release();
		m_pNextCmdTarg = nullptr;
	}
}

// 设置命令链中的下一个目标
// pNextCmdTargFromView 是从 IVsTextView::AddCommandFilter 获取的，它已经被 AddRef'd
void CCommandFilter::SetNextTarget(IOleCommandTarget* pNextCmdTargFromView)
{
	if (m_pNextCmdTarg)
	{
		m_pNextCmdTarg->Release(); // 释放之前持有的目标
	}
	m_pNextCmdTarg = pNextCmdTargFromView; // 获取新的目标的所有权
	// 不需要在这里 AddRef，因为我们接管了 pNextCmdTargFromView 已有的引用计数
}

// IUnknown 实现
STDMETHODIMP CCommandFilter::QueryInterface(REFIID riid, void** ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	*ppvObject = nullptr;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IOleCommandTarget))
	{
		*ppvObject = static_cast<IOleCommandTarget*>(this);
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CCommandFilter::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CCommandFilter::Release()
{
	ULONG ulRefCount = InterlockedDecrement(&m_cRef);
	if (0 == ulRefCount)
	{
		delete this;
	}
	return ulRefCount;
}

// IOleCommandTarget 实现: QueryStatus
STDMETHODIMP CCommandFilter::QueryStatus(
	const GUID* pguidCmdGroup,
	ULONG cCmds,
	OLECMD prgCmds[],
	OLECMDTEXT* pCmdText)
{
	if (!pguidCmdGroup || !prgCmds)
		return E_INVALIDARG;

	bool handledByUs = false;

// 	if (IsEqualGUID(*pguidCmdGroup, CMDSETID_Standard2K))
// 	{
// 		for (ULONG i = 0; i < cCmds; i++)
// 		{
// 			prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
// 			handledByUs = true;
// 		}
// 	}
// 
// 	if (IsEqualGUID(*pguidCmdGroup, CGID_VSStandardCommandSet97))
// 	{
// 		for (ULONG i = 0; i < cCmds; i++)
// 		{
// 			prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
// 			handledByUs = true;
// 		}
// 	}

// 
// 
// 	// 检查是否是我们关心的命令组 (例如：标准编辑器命令)
// 	if (IsEqualGUID(*pguidCmdGroup, CMDSETID_Standard2K))
// 	{
// 		for (ULONG i = 0; i < cCmds; i++)
// 		{
// 			// 检查是否是我们关心的特定命令
// 			switch (prgCmds[i].cmdID)
// 			{
// 			case VSStd2KCmdID_Example::ECMD_TYPECHAR_EXAMPLE:
// 			case VSStd2KCmdID_Example::ECMD_PASTE_EXAMPLE:
// 			case VSStd2KCmdID_Example::ECMD_DELETE_EXAMPLE:
// 			case VSStd2KCmdID_Example::ECMD_BACKSPACE_EXAMPLE:
// 				// 表明我们支持并启用了这些命令
// 				prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
// 				handledByUs = true;
// 
// 				// (可选) 如果 pCmdText 非空，可以设置命令的文本描述
// 				if (pCmdText && pCmdText->cmdtextf == OLECMDTEXTF_NAME) {
// 					// const wchar_t* szName = L"My Custom Action";
// 					// wcsncpy_s(pCmdText->rgwz, pCmdText->cwBuf, szName, _TRUNCATE);
// 					// pCmdText->cwActual = static_cast<ULONG>(wcslen(szName));
// 				}
// 				break;
// 			default:
// 				// 对于此命令组中的其他命令，我们不显式处理
// 				break;
// 			}
// 		}
// 	}

	// 如果我们处理了任何命令，返回 S_OK
	if (handledByUs)
	{
		return S_OK;
	}

	// 否则，将查询传递给命令链中的下一个目标
	if (m_pNextCmdTarg)
	{
		return m_pNextCmdTarg->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
	}

	// 如果没有下一个目标或我们不处理该命令组
	return OLECMDERR_E_UNKNOWNGROUP; // 或者 OLECMDERR_E_NOTSUPPORTED
}

// IOleCommandTarget 实现: Exec
STDMETHODIMP CCommandFilter::Exec(
	const GUID* pguidCmdGroup,
	DWORD nCmdID,
	DWORD nCmdexecopt,
	VARIANT* pvaIn,
	VARIANT* pvaOut)
{
	if (!pguidCmdGroup)
		return E_INVALIDARG;

	// 拦截 LazyBug 自定义命令：diff 导航
	if (IsEqualGUID(*pguidCmdGroup, guidLazyBugCmdSet))
	{
		if (nCmdID == LazyBugNextDiff)
		{
			Util_NavigateNextDiff(g_fileChangeAttach._filePath, g_fileChangeAttach._comparingContent, true, false);
			return S_OK;
		}
		else if (nCmdID == LazyBugPrevDiff)
		{
			Util_NavigateNextDiff(g_fileChangeAttach._filePath, g_fileChangeAttach._comparingContent, false, false);
			return S_OK;
		}
	}

	bool commandBlockedByUs = false;

	if (IsEqualGUID(*pguidCmdGroup, CGID_VSStandardCommandSet97))
	{
		switch (nCmdID)
		{
			case cmdidSave:
			case cmdidSaveAs:
			case cmdidSaveProjectItem:
				commandBlockedByUs = true;
				break;
		}
	}


	// 检查是否是我们关心的命令组
	if (IsEqualGUID(*pguidCmdGroup, CMDSETID_Standard2K))
	{
		switch (nCmdID)
		{
		case ECMD_LEFT:
		case ECMD_RIGHT:
		case ECMD_UP:
		case ECMD_DOWN:
		case ECMD_HOME:
		case ECMD_END:
		case ECMD_PAGEUP:
		case ECMD_PAGEDN:
		case ECMD_TOPLINE:
		case ECMD_BOTTOMLINE:
		case ECMD_SCROLLUP:
		case ECMD_SCROLLDN:
		case ECMD_SCROLLPAGEUP:
		case ECMD_SCROLLPAGEDN:
		case ECMD_SCROLLLEFT:
		case ECMD_SCROLLRIGHT:
		case ECMD_SCROLLBOTTOM:
		case ECMD_SCROLLCENTER:
		case ECMD_SCROLLTOP:
		case ECMD_SELECTALL:
		case ECMD_GOTOLINE:
		case ECMD_GOTOBRACE:
		case ECMD_GOTOBRACE_EXT:
		case ECMD_COPY:
		case ECMD_GOTONEXTBOOKMARK:
		case ECMD_GOTOPREVBOOKMARK:
		case ECMD_FIND:
		case ECMD_FINDNEXT:
		case ECMD_FINDNEXTWORD:
		case ECMD_FINDPREV:
		case ECMD_FINDPREVWORD:
		case ECMD_FINDAGAIN:
		case ECMD_SELECTCURRENTWORD:
		case ECMD_WORDPREV:
		case ECMD_WORDNEXT:
			break;
		default:
			commandBlockedByUs = true;
			break;
		}

		switch (nCmdID) 
		{
		case ECMD_TYPECHAR:
		case ECMD_BACKSPACE:
		case ECMD_RETURN:
		case ECMD_TAB:
		case ECMD_BACKTAB:
		case ECMD_DELETE:
		case ECMD_PASTE:
		case ECMD_CUTLINE:
		case ECMD_DELETELINE:
		case ECMD_DELETEBLANKLINES:
		case ECMD_DELETEWHITESPACE:
		case ECMD_DELETETOEOL:
		case ECMD_DELETETOBOL:
		case ECMD_CANCEL: // Esc 键退出 diff 模式
			g_ps.requestDetachFileChange = true;
			break;
		}

	}

	// 如果我们阻止了命令，则返回 S_OK，不再传递给下一个目标
	if (commandBlockedByUs)
	{
		return S_OK;
	}

	// 否则，将命令传递给命令链中的下一个目标
	if (m_pNextCmdTarg)
	{
		return m_pNextCmdTarg->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvaIn, pvaOut);
	}

	// 如果没有下一个目标或我们不处理该命令组
	return OLECMDERR_E_NOTSUPPORTED; // 或者 OLECMDERR_E_UNKNOWNGROUP
}

// 自定义剪贴板格式 ID（LazyBugSelectionRef）
UINT LazyBug_GetSelectionRefClipboardFormat()
{
	static UINT s_cf = RegisterClipboardFormatW(L"LazyBugSelectionRef");
	return s_cf;
}

// 在默认复制完成后，将选中引用信息（JSON）以自定义格式追加到剪贴板
void LazyBug_AppendSelectionRefToClipboard()
{
	if (!g_ps.pTextManager) return;

	CComPtr<IVsTextView> pView;
	if (FAILED(g_ps.pTextManager->GetActiveView(FALSE, NULL, &pView)) || !pView)
		return;

	long iStartLine = 0, iStartIndex = 0, iEndLine = 0, iEndIndex = 0;
	if (FAILED(pView->GetSelection(&iStartLine, &iStartIndex, &iEndLine, &iEndIndex)))
		return;

	bool bHasSelection = !(iStartLine == iEndLine && iStartIndex == iEndIndex);

	if (!bHasSelection)
	{
		// 无选中文本：复制当前行为引用信息
	}
	else if (iStartLine == iEndLine)
	{
		// 有选中但在同一行上：不复制引用信息
		return;
	}

	// GetSelection 可能是反向选择（anchor 在后），规整起止行顺序
	if (iStartLine > iEndLine)
	{
		long t = iStartLine; iStartLine = iEndLine; iEndLine = t;
	}

	// 无选中时 endLine 已等于 startLine，规整后 JSON 的 startLine/endLine 相同

	CComPtr<IVsTextLines> pBuffer;
	if (FAILED(pView->GetBuffer(&pBuffer)) || !pBuffer)
		return;

	CComQIPtr<IPersistFileFormat> pPersistFileFormat(pBuffer);
	if (!pPersistFileFormat)
		return;

	LPOLESTR pszFilename = NULL;
	DWORD formatIndex = 0;
	if (FAILED(pPersistFileFormat->GetCurFile(&pszFilename, &formatIndex)) || !pszFilename)
		return;

	CString strPath(pszFilename);
	::CoTaskMemFree(pszFilename);

	// 生成 JSON
	CT2CA pszUtf8(strPath, CP_UTF8);
	nlohmann::json j;
	j["file"] = (const char*)pszUtf8;
	j["startLine"] = iStartLine + 1;
	j["endLine"] = iEndLine + 1;
	std::string jsonStr = j.dump();

	// 追加写入剪贴板（不清空，保留 CF_UNICODETEXT）
	if (OpenClipboard(NULL))
	{
		int wideLen = MultiByteToWideChar(CP_UTF8, 0, jsonStr.c_str(), -1, NULL, 0);
		HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, wideLen * sizeof(WCHAR));
		if (hGlob)
		{
			WCHAR* pWide = (WCHAR*)GlobalLock(hGlob);
			if (pWide)
			{
				MultiByteToWideChar(CP_UTF8, 0, jsonStr.c_str(), -1, pWide, wideLen);
				GlobalUnlock(hGlob);
				SetClipboardData(LazyBug_GetSelectionRefClipboardFormat(), hGlob);
			}
			else
			{
				GlobalFree(hGlob);
			}
		}
		CloseClipboard();
	}
}


// ========== CCopyRefCommandFilter ==========

STDMETHODIMP CCopyRefCommandFilter::QueryInterface(REFIID riid, void** ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IOleCommandTarget))
	{
		*ppvObject = static_cast<IOleCommandTarget*>(this);
		AddRef();
		return S_OK;
	}
	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CCopyRefCommandFilter::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CCopyRefCommandFilter::Release()
{
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (cRef == 0)
		delete this;
	return cRef;
}

STDMETHODIMP CCopyRefCommandFilter::QueryStatus(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT* pCmdText)
{
	if (m_pNextCmdTarg)
		return m_pNextCmdTarg->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
	return OLECMDERR_E_NOTSUPPORTED;
}

STDMETHODIMP CCopyRefCommandFilter::Exec(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT* pvaIn, VARIANT* pvaOut)
{
	if (!pguidCmdGroup)
		return E_INVALIDARG;

	// 只拦截标准复制命令：先执行默认复制，成功后追加自定义格式
	// 编辑器的复制命令属于 VSStd97 命令集（cmdidCopy=15），而非 Standard2K
	bool isCopy =
		(IsEqualGUID(*pguidCmdGroup, CGID_VSStandardCommandSet97) && nCmdID == cmdidCopy) ||
		(IsEqualGUID(*pguidCmdGroup, CMDSETID_Standard2K) && nCmdID == ECMD_COPY);
	if (isCopy && m_pNextCmdTarg)
	{
		HRESULT hr = m_pNextCmdTarg->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvaIn, pvaOut);
		if (SUCCEEDED(hr))
			LazyBug_AppendSelectionRefToClipboard();
		return hr;
	}

	// 其它命令一律透传
	if (m_pNextCmdTarg)
		return m_pNextCmdTarg->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvaIn, pvaOut);

	return OLECMDERR_E_NOTSUPPORTED;
}


// ========== CTextViewCreationListener ==========

HRESULT CTextViewCreationListener::Advise(IVsTextManager* pTextManager)
{
	if (!pTextManager) return E_POINTER;
	if (m_dwCookie != 0) return S_FALSE; // 已注册

	m_pTextManager = pTextManager;

	CComQIPtr<IConnectionPointContainer> pCPC(pTextManager);
	if (!pCPC)
		return E_NOINTERFACE;

	HRESULT hr = pCPC->FindConnectionPoint(IID_IVsTextManagerEvents, &m_pConnectionPoint);
	if (FAILED(hr) || !m_pConnectionPoint)
		return hr;

	return m_pConnectionPoint->Advise(static_cast<IVsTextManagerEvents*>(this), &m_dwCookie);
}

HRESULT CTextViewCreationListener::Unadvise()
{
	if (m_dwCookie != 0 && m_pConnectionPoint)
	{
		m_pConnectionPoint->Unadvise(m_dwCookie);
		m_dwCookie = 0;
	}
	m_pConnectionPoint.Release();
	m_pTextManager.Release();
	return S_OK;
}

STDMETHODIMP_(void) CTextViewCreationListener::OnRegisterView(IVsTextView* pView)
{
	if (!pView) return;

	// 给新建的编辑器视图安装轻量复制过滤器
	CCopyRefCommandFilter* pFilter = new CCopyRefCommandFilter();
	IOleCommandTarget* pNext = nullptr;
	if (SUCCEEDED(pView->AddCommandFilter(pFilter, &pNext)))
		pFilter->SetNextTarget(pNext);
	// AddCommandFilter 已持有引用，释放本地引用
	pFilter->Release();
}

STDMETHODIMP_(void) CTextViewCreationListener::OnUnregisterView(IVsTextView* pView)
{
	// 视图销毁时命令链自动释放过滤器，无需额外处理
}