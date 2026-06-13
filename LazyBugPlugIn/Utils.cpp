#include "stdh.h" // Common header, adjust if your project uses a different one
#include <vsshell.h> // For IVsShell, IVsUIShell5 etc.

// UTF-8 转宽字符函数声明
extern std::wstring utf8_to_widechar(const char* utf8_str);
#include <textmgr.h> // For IVsTextLines, IVsTextLineMarker, marker types, TBS_READONLY

#include "Utils.h"

#include "codediff/CodeDiff.h"
#include "stringparser/stringparser.h"
#include "CommandFilter.h"
#include "DocReloadFilter.h"

#include "RDTEventsListener.h"

#include "../Proj_LazyBug/SolutionDump.h"

#include <vssolutn.h> // For IVsSolution
#include <dte.h>      // For DTE
#include <dte80.h>    // For DTE2

// VCProject / VCConfiguration 接口（VC++ 项目属性）
// 需要 #import 或手动声明，这里用 DTE2 automation 模型通过 IDispatch 访问
#include <comutil.h>  // _bstr_t, _variant_t

#include <fstream>
#include <sstream>    // std::wistringstream
#include <shlwapi.h>  // PathIsRelativeW, PathCombineW, PathCanonicalizeW
#pragma comment(lib, "shlwapi.lib")

// 辅助函数：设置文本视图的顶部行
static void TextViewSetTopLine(IVsTextView* pTextView, long line)
{
	// 获取文本缓冲区
	CComPtr<IVsTextLines> pTextLines;
	if (FAILED(pTextView->GetBuffer(&pTextLines)) || !pTextLines)
	{
		return;
	}

	// 获取目标行的位置
	long lineLength = 0;
	if (FAILED(pTextLines->GetLengthOfLine(line, &lineLength)))
	{
		return;
	}

	// 创建文本范围
	TextSpan ts;
	ts.iStartLine = line;
	ts.iStartIndex = 0;
	ts.iEndLine = line + 10;
	ts.iEndIndex = 0;

	// 确保行可见
	pTextView->EnsureSpanVisible(ts);
}

HRESULT Util_IsFileOpenInEditor(const wchar_t* filePath)
{
	if (!filePath || filePath[0] == L'\0')
	{
		return E_INVALIDARG;
	}

	CComPtr<IServiceProvider> sp = g_ps.pServiceProvider;
	if (!sp)
	{
		return E_UNEXPECTED;
	}

	CComPtr<IVsRunningDocumentTable> pRDT;
	HRESULT hr = sp->QueryService(SID_SVsRunningDocumentTable, IID_IVsRunningDocumentTable, (void**)&pRDT);
	if (FAILED(hr) || !pRDT)
	{
		return hr;
	}

	CComPtr<IVsHierarchy> pHier;
	VSITEMID itemid = VSITEMID_NIL;
	CComPtr<IUnknown> pDocDataUnk;
	VSCOOKIE docCookie = VSCOOKIE_NIL;

	// 在RDT中查找文档
	hr = pRDT->FindAndLockDocument(
		(DWORD)_VSRDTFLAGS::RDT_NoLock,
		filePath,
		&pHier,
		&itemid,
		&pDocDataUnk,
		&docCookie
	);

	if (FAILED(hr) || docCookie == VSCOOKIE_NIL || !pDocDataUnk)
	{
		return S_FALSE; // 文档未打开
	}

	// 检查文档是否在编辑器中打开
	CComPtr<IVsUIShellOpenDocument> pUiShellOpenDoc;
	hr = sp->QueryService(SID_SVsUIShellOpenDocument, IID_IVsUIShellOpenDocument, (void**)&pUiShellOpenDoc);
	if (FAILED(hr) || !pUiShellOpenDoc)
	{
		return S_FALSE; // 无法获取服务，保守返回未打开
	}

	CComPtr<IVsUIHierarchy> pUiHier;
	if (pHier)
	{
		pHier->QueryInterface(IID_IVsUIHierarchy, (void**)&pUiHier);
	}

	int pfOpen = 0;
	CComPtr<IVsWindowFrame> pWindowFrame;
	GUID logicalView = LOGVIEWID_Primary;

	hr = pUiShellOpenDoc->IsDocumentOpen(
		pUiHier,
		itemid,
		filePath,
		logicalView,
		0,
		nullptr,
		nullptr,
		&pWindowFrame,
		&pfOpen
	);

	if (SUCCEEDED(hr) && pfOpen && pWindowFrame)
	{
		return S_OK; // 文档在编辑器中打开
	}

	return S_FALSE; // 文档未在编辑器中打开
}

HRESULT Util_OpenFileInEditor(const wchar_t* filePath, int line)
{
	IServiceProvider* pServiceProvider = g_ps.pServiceProvider;

	if (!pServiceProvider || !filePath || filePath[0] == '\0')
	{
		return E_FAIL;
	}

// 	IVsUIShellOpenDocument* pOpenDoc = nullptr;
// 	HRESULT hr = pServiceProvider->QueryService(SID_SVsUIShellOpenDocument,
// 		IID_IVsUIShellOpenDocument,
// 		(void**)&pOpenDoc);
// 	if (FAILED(hr) || !pOpenDoc)
// 	{
// 		// 处理错误，例如记录日志或显示消息
// 		if (pOpenDoc) pOpenDoc->Release();
// 		return hr;
// 	}

	CComPtr< IVsUIShellOpenDocument> pOpenDoc = g_ps.pUIShellOpenDocument;
	if (!pOpenDoc)
		return E_FAIL;

	CComPtr < IVsWindowFrame> pWindowFrame;

	// LOGVIEWID_Primary 通常表示文件的默认视图（例如，代码编辑器用于 .cpp 文件）
	// LOGVIEWID_Code 明确请求代码视图
	// LOGVIEWID_TextView 明确请求文本视图
	// GUID_NULL 可以用于 rguidLogicalView，让VS决定最佳视图
	GUID logicalView = LOGVIEWID_Code; // 或者 LOGVIEWID_Code, LOGVIEWID_TextView

// 	IServiceProvider* pServiceProvider2 = nullptr;
// 	IVsUIHierarchy* pHierarchy = nullptr;
// 	VSITEMID itemId;

	HRESULT hr;
	hr = pOpenDoc->OpenDocumentViaProject(filePath, logicalView, nullptr, nullptr, nullptr, &pWindowFrame);
// 
// 	// 打开文件
// 	hr = pOpenDoc->OpenStandardEditor(
// 		0,                      // OSE_DEFAULT, 或者其他标志如 OSE_OpenAsNewFile
// 		filePath,               // 文件的完整路径
// 		logicalView,            // 逻辑视图的GUID
// 		L"",                // [in] 文档所有者的标题 (通常为 nullptr)
// 		nullptr,                // [in] IVsUIHierarchy* pUIH (如果文件属于特定项目层次结构)
// 		VSITEMID_NIL,           // [in] VSITEMID itemid (如果文件属于特定项目层次结构)
// 		nullptr,
// 		pServiceProvider,                 // [in] IOleServiceProvider* psp (用于新编辑器的站点)
// 		&pWindowFrame           // [out] 指向文档窗口框架的指针
// 	);


	if (SUCCEEDED(hr) && pWindowFrame)
	{
		hr = pWindowFrame->Show();
		
		// 如果指定了行号，移动光标到该行
		if (SUCCEEDED(hr) && line >= 0)
		{
			// 获取文本视图
			CComPtr<IVsTextView> pTextView = Util_GetTextViewForFile(filePath);
			if (pTextView)
			{
				// 设置光标位置（line是0-based）
				pTextView->SetCaretPos(line, 0);
				// 确保该行可见
				TextSpan ts;
				ts.iStartLine = line;
				ts.iStartIndex = 0;
				ts.iEndLine = line;
				ts.iEndIndex = 0;
				pTextView->EnsureSpanVisible(ts);
			}
		}
	}
	else if (FAILED(hr))
	{
		// 处理打开文件失败的情况
		// 例如，显示错误消息给用户
	}

	return hr;
}

// 设置文本视图焦点的辅助函数
void Util_SetTextViewFocus(CComPtr<IVsTextView> pTextView)
{
	if (!pTextView)
	{
		return;
	}

	// 方法1: 通过IVsTextView直接设置焦点
	CComPtr<IVsWindowPane> pWindowPane;
	HRESULT hr = pTextView->QueryInterface(IID_IVsWindowPane, (void**)&pWindowPane);
	if (SUCCEEDED(hr) && pWindowPane)
	{
		// 获取窗口句柄并设置焦点
		HWND hwnd = NULL;
		hr = pWindowPane->GetDefaultSize(NULL); // 这会确保窗口已创建
		if (SUCCEEDED(hr))
		{
			// 尝试获取窗口句柄
			CComPtr<IOleInPlaceActiveObject> pActiveObject;
			hr = pWindowPane->QueryInterface(IID_IOleInPlaceActiveObject, (void**)&pActiveObject);
			if (SUCCEEDED(hr) && pActiveObject)
			{
				hr = pActiveObject->GetWindow(&hwnd);
				if (SUCCEEDED(hr) && hwnd)
				{
					::SetFocus(hwnd);
					return;
				}
			}
		}
	}

	// 方法2: 通过IVsWindowFrame设置焦点
	CComPtr<IServiceProvider> sp = g_ps.pServiceProvider;
	if (!sp)
	{
		return;
	}

	CComPtr<IVsUIShell> pUIShell;
	hr = sp->QueryService(SID_SVsUIShell, IID_IVsUIShell, (void**)&pUIShell);
	if (FAILED(hr) || !pUIShell)
	{
		return;
	}

	// 查找包含此文本视图的窗口框架
	CComPtr<IEnumWindowFrames> pEnum;
	hr = pUIShell->GetDocumentWindowEnum(&pEnum);
	if (SUCCEEDED(hr) && pEnum)
	{
		CComPtr<IVsWindowFrame> pFrame;
		ULONG fetched = 0;

		while (pEnum->Next(1, &pFrame, &fetched) == S_OK && fetched == 1)
		{
			// 检查这个窗口框架是否包含我们的文本视图
			VARIANT var;
			VariantInit(&var);
			hr = pFrame->GetProperty((int)__VSFPROPID::VSFPROPID_DocView, &var);

			if (SUCCEEDED(hr) && var.vt == VT_UNKNOWN && var.punkVal)
			{
				CComPtr<IVsTextView> pFrameTextView;

				// 直接查询IVsTextView
				hr = var.punkVal->QueryInterface(IID_IVsTextView, (void**)&pFrameTextView);
				if (FAILED(hr))
				{
					// 如果不是直接的IVsTextView，可能是IVsCodeWindow
					CComPtr<IVsCodeWindow> pCodeWindow;
					hr = var.punkVal->QueryInterface(IID_IVsCodeWindow, (void**)&pCodeWindow);
					if (SUCCEEDED(hr) && pCodeWindow)
					{
						hr = pCodeWindow->GetPrimaryView(&pFrameTextView);
					}
				}

				// 检查是否是我们要找的文本视图
				if (SUCCEEDED(hr) && pFrameTextView && pFrameTextView.IsEqualObject(pTextView))
				{
					// 找到了对应的窗口框架，设置为活动窗口
					pFrame->Show();
					VariantClear(&var);
					return;
				}
			}

			VariantClear(&var);
			pFrame.Release();
		}
	}
}


void Util_NavigateNextDiff(const std::wstring& filePath, const CodeComparingLines& content, bool isNext,bool allowCycle)
{
	// 获取文件的文本视图
	CComPtr<IVsTextView> pTextView = Util_GetTextViewForFile(filePath);
	if (!pTextView)
	{
		return;
	}

	// 获取当前光标位置
	long currentLine = 0;
	long currentColumn = 0;
	HRESULT hr = pTextView->GetCaretPos(&currentLine, &currentColumn);
	if (FAILED(hr))
	{
		return;
	}

	// 查找下一个差异行
	int targetLine = content.NavigateDiff(currentLine, isNext);
	if (targetLine < 0)
	{
		if (allowCycle && isNext)
			targetLine = content.NavigateDiff(0, isNext);
		if (targetLine < 0)
			return; // 未找到符合条件的行
	}

	// 确保目标行可见
	TextViewSetTopLine(pTextView, targetLine);

	// 设置光标位置
	pTextView->SetCaretPos(targetLine, 0);

	// 设置窗口焦点
	Util_SetTextViewFocus(pTextView);
}


CComPtr<IVsTextView> Util_GetTextViewForFile(const std::wstring& filePath)
{
	CComPtr<IServiceProvider>sp = g_ps.pServiceProvider;
	if (!sp)
	{
		// Service provider is essential
		return nullptr;
	}

	CComPtr<IVsUIShellOpenDocument> pUiShellOpenDoc = g_ps.pUIShellOpenDocument;
	if (!pUiShellOpenDoc)
	{
		return nullptr;
	}


	CComPtr<IVsRunningDocumentTable> pRDT;
	HRESULT hr = sp->QueryService(SID_SVsRunningDocumentTable, IID_IVsRunningDocumentTable, (void**)&pRDT);
	if (FAILED(hr) || !pRDT)
	{
		return nullptr;
	}

	CComPtr<IVsHierarchy> pHier;
	VSITEMID itemid = VSITEMID_NIL;
	CComPtr<IUnknown> pDocDataUnk; // This will be an IUnknown pointer to the document's data object
	VSCOOKIE docCookie = VSCOOKIE_NIL;

	// 1. 在 RDT 中查找文档以获取其 cookie 和可能的层级信息
	// RDT_NoLock 表示我们只是查找，不锁定文档进行编辑
	// 如果文件未在RDT中（即未打开或未被VS以任何形式跟踪），这将失败
	hr = pRDT->FindAndLockDocument(
		(DWORD)_VSRDTFLAGS::RDT_NoLock, // 或者 RDT_ReadLock 如果你需要确保文档数据有效
		filePath.c_str(),
		&pHier, // 输出: 文档所属的层级 (例如项目)
		&itemid, // 输出: 文档在层级中的ID
		&pDocDataUnk, // 输出: 文档数据对象 (通常是 IVsTextBuffer 的 IUnknown)
		&docCookie  // 输出: 文档在RDT中的唯一标识符
	);

	// FindAndLockDocument 会 AddRef pHier 和 pDocDataUnk，CComPtr 会自动处理 Release

	if (FAILED(hr) || docCookie == VSCOOKIE_NIL)
	{
		// 文档不在RDT中，或者查找时出错
		return nullptr;
	}

	CComPtr<IVsUIHierarchy> pUiHier; // IsDocumentOpen 需要 IVsUIHierarchy
	if (SUCCEEDED(hr) && pHier) // 如果 FindAndLockDocument 成功并且返回了有效的 pHier
	{
		// 尝试从 IVsHierarchy 获取 IVsUIHierarchy 接口
		hr = pHier->QueryInterface(IID_IVsUIHierarchy, (void**)&pUiHier);
		if (FAILED(hr))
		{
			pUiHier = nullptr; // 获取失败，则传递 nullptr
		}
	}

	CComPtr<IVsWindowFrame> pWindowFrame;
	int pfOpen = 0; // 输出参数，指示文档是否真的在编辑器中打开

	GUID logicalView = LOGVIEWID_Primary; // 或者 LOGVIEWID_Code, LOGVIEWID_TextView

    //  检查文档是否在编辑器窗口中打开
    // IsDocumentOpen 可以接受 nullptr 的 IVsUIHierarchy，此时它会更多地依赖 pszMkDocument。
    // 如果 pUiHier 和 itemid 有效，它们将被优先使用。
    hr = pUiShellOpenDoc->IsDocumentOpen(
        pUiHier,          // [in] IVsUIHierarchy (可以为 nullptr)
        itemid,           // [in] VSITEMID (如果 pUiHier 为 nullptr，则此参数通常也应为 VSITEMID_NIL 或被忽略)
        filePath.c_str(), // [in] 文档的 Moniker (完整路径)
        logicalView,      // [in] 逻辑视图的 GUID
        0,                // [in] grfIDO (打开标志，通常为0)
        nullptr,          // [out] ppHierActivated (可选输出)
        nullptr,          // [out] pitemidActivated (可选输出)
        &pWindowFrame,    // [out] 文档的窗口框架
        &pfOpen           // [out] 指示文档是否在指定视图中打开
    );

	if (FAILED(hr) || pWindowFrame == nullptr)
	{
		return nullptr;
	}


	// 从 IVsWindowFrame 获取 IVsTextView
	CComPtr<IVsTextView> pTextView;
	VARIANT var;
	VariantInit(&var);

	// VSFPROPID_DocView 通常返回视图对象 (例如，对于文本编辑器，这可能是 IVsCodeWindow)
	// DocView 可能是一个更通用的对象 (如 IVsCodeWindow)，需要进一步查询
	hr = pWindowFrame->GetProperty((int)__VSFPROPID::VSFPROPID_DocView, &var);
	if (SUCCEEDED(hr) && var.vt == VT_UNKNOWN && var.punkVal != nullptr)
	{
		CComPtr<IUnknown> pUnkDocView = var.punkVal;
		VariantClear(&var);

		// 尝试直接从 DocView 获取 IVsTextView
		hr = pUnkDocView->QueryInterface(IID_IVsTextView, (void**)&pTextView);
		if (FAILED(hr) || !pTextView)
		{
			// 如果 DocView 本身不是 IVsTextView (例如它是 IVsCodeWindow)
			// 你可能需要从 IVsCodeWindow 获取主视图或辅助视图
			CComPtr<IVsCodeWindow> pCodeWindow;
			if (SUCCEEDED(pUnkDocView->QueryInterface(IID_IVsCodeWindow, (void**)&pCodeWindow)) && pCodeWindow)
			{
				hr = pCodeWindow->GetPrimaryView(&pTextView); // 或者 GetSecondaryView
				// pTextView CComPtr 将管理其引用计数
			}
		}
	}
	else
	{
		VariantClear(&var);
	}

	return pTextView; // 如果未找到或任何步骤失败，则返回 nullptr
}

void Util_ClearUserData(CComPtr<IVsTextView> pVsTextView)
{
	if (!pVsTextView)
	{
		return;
	}

	CComPtr<IVsTextLines> pTextLines;
	HRESULT hr = pVsTextView->GetBuffer(&pTextLines);
	if (FAILED(hr) || !pTextLines)
	{
		return;
	}

	CComPtr<IVsUserData> pUserData;
	if (FAILED(pTextLines->QueryInterface(IID_IVsUserData, (void**)&pUserData)) || !pUserData)
	{
		return;
	}

	// 使用与 StoreDiffInUserData 中相同的 GUID
	// {257A16BC-9B4E-4C67-8528-EE23A740BAD4}
	static GUID guidDiffData = { 0x257a16bc, 0x9b4e, 0x4c67, { 0x85, 0x28, 0xee, 0x23, 0xa7, 0x40, 0xba, 0xd4 } };

	CComVariant vtEmpty; // 默认构造为 VT_EMPTY
	pUserData->SetData(guidDiffData, vtEmpty); 
}

// {A1B2C3D4-E5F6-7890-ABCD-EF1234567890} - FileChange border flag GUID
static GUID guidFileChangeBorder = { 0xa1b2c3d4, 0xe5f6, 0x7890, { 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x90 } };

void Util_SetFileChangeBorder(CComPtr<IVsTextView> pVsTextView, bool show)
{
	if (!pVsTextView)
	{
		return;
	}

	CComPtr<IVsTextLines> pTextLines;
	HRESULT hr = pVsTextView->GetBuffer(&pTextLines);
	if (FAILED(hr) || !pTextLines)
	{
		return;
	}

	CComPtr<IVsUserData> pUserData;
	if (FAILED(pTextLines->QueryInterface(IID_IVsUserData, (void**)&pUserData)) || !pUserData)
	{
		return;
	}

	if (show)
	{
		CComVariant vtTrue(true);
		pUserData->SetData(guidFileChangeBorder, vtTrue);
	}
	else
	{
		CComVariant vtEmpty; // VT_EMPTY 表示清除
		pUserData->SetData(guidFileChangeBorder, vtEmpty);
	}
}

static void StoreDiffInUserData(IVsTextLines* pVsTextLines, const CodeComparingLines& comparaingContent)
{
	if (!pVsTextLines) return;

	// {257A16BC-9B4E-4C67-8528-EE23A740BAD4}
	static GUID guid= { 0x257a16bc, 0x9b4e, 0x4c67, { 0x85, 0x28, 0xee, 0x23, 0xa7, 0x40, 0xba, 0xd4 } };

	CComPtr<IVsUserData> pUserData;
	if (FAILED(pVsTextLines->QueryInterface(IID_IVsUserData, (void**)&pUserData)) || !pUserData)
	{
		return;
	}

	if (comparaingContent.lineOrigins.empty())
	{
		CComVariant vtEmpty; // VT_EMPTY by default
		pUserData->SetData(guid, vtEmpty);
		return;
	}

	// 1. 创建外层 SAFEARRAY (VT_VARIANT)，大小为 map 的条目数
	CComSafeArray<VARIANT> saOuter;

	for (int i=0;i<comparaingContent.lineOrigins.size();i++)
	{
		long lineNumber = i;
		int value = (int)comparaingContent.lineOrigins[i].tp;

		if (comparaingContent.lineOrigins[i].tp == CodeComparingLines::LineOrigin::Both)
			continue;

		// 2. 为每个键值对创建内层 SAFEARRAY (VT_VARIANT), 大小为 2
		//    或者可以直接用 VT_I4，但 VT_VARIANT 更灵活，因为 CComVariant 会处理转换
		CComSafeArray<VARIANT> saInner(2UL); // 2 elements: [lineNumber, value]
		if (!saInner.m_psa)
		{
			// 处理错误，可能需要销毁 saOuter
			return;
		}

		CComVariant vtKey(lineNumber); // long -> VT_I4 or VT_R8 if too large, CComVariant handles
		CComVariant vtValue(value);    // int -> VT_I4

		saInner.SetAt(0, vtKey, false); // false = don't copy variant, CComVariant handles lifetime
		saInner.SetAt(1, vtValue, false);

		// 3. 将内层 SAFEARRAY 包装在 VARIANT 中
		CComVariant vtPair;
		vtPair.vt = VT_ARRAY | VT_VARIANT;
		vtPair.parray = saInner.Detach(); // Detach to transfer ownership to vtPair

		// 4. 将此 VARIANT (vtPair) 添加到外层 SAFEARRAY
		saOuter.Add(vtPair);
	}

	// 5. 将外层 SAFEARRAY 包装在最终的 VARIANT 中
	CComVariant varMapData;
	varMapData.vt = VT_ARRAY | VT_VARIANT;
	varMapData.parray = saOuter.Detach(); // Transfer ownership of saOuter to varMapData

	// 6. 存储到 UserData
	pUserData->SetData(guid, varMapData);
	// varMapData will be cleared on destruction, releasing the SAFEARRAYs.
}

bool Util_SetComparingContent(const std::wstring& filePath, CComPtr<IVsTextView> pVsTextView, const CodeComparingLines& comparaingContent)
{
	if (!pVsTextView)
	{
		return false;
	}

	CComPtr<IVsTextLines> pTextLines;
	HRESULT hr = pVsTextView->GetBuffer(&pTextLines);
	if (FAILED(hr) || !pTextLines)
	{
		return false;
	}

	DWORD flags = 0;
	if (TRUE)
	{
		pTextLines->GetStateFlags(&flags);
	}

	if (flags & BSF_FILESYS_READONLY)
		return false;

	// 先解除只读状态，以便修改内容
	pTextLines->SetStateFlags(flags&(~BSF_USER_READONLY));

	std::wstring wContent = utf8_to_widechar(comparaingContent.content.c_str());
	const wchar_t* pszNewText = wContent.c_str();
	int iNewLength = static_cast<int>(wContent.length());

	// 获取旧内容的结束位置
	long iOldLineCount = 0;
	hr = pTextLines->GetLineCount(&iOldLineCount);
	if (FAILED(hr))
		return false; // 无法获取行数

	long iOldEndIndex = 0;
	if (iOldLineCount > 0)
	{
		hr = pTextLines->GetLengthOfLine(iOldLineCount - 1, &iOldEndIndex);
		if (FAILED(hr))
		{
			iOldEndIndex = 0; // 出错则认为长度为0
		}
	}

	// 替换整个缓冲区内容
	long endLine = (iOldLineCount > 0) ? iOldLineCount - 1 : 0;
	hr = pTextLines->ReplaceLines(0, 0, endLine, iOldEndIndex, pszNewText, iNewLength, nullptr);

	if (FAILED(hr))
		return false; // 替换失败
	StoreDiffInUserData(pTextLines, comparaingContent);

  	pTextLines->SetStateFlags(flags&~BSF_MODIFIED);

 	pVsTextView->SetCaretPos(0, 0);

	return SUCCEEDED(hr);
}

CCommandFilter* Util_AddCommandFilter(CComPtr<IVsTextView> pVsTextView)
{
	// 1. 创建你的命令过滤器实例
	CCommandFilter* pFilter = new CCommandFilter(); // 初始引用计数为 1

	// 2. 将过滤器添加到 IVsTextView 的命令链中
	IOleCommandTarget* pNextCmdTargetFromView = nullptr;
	HRESULT hr = pVsTextView->AddCommandFilter(pFilter, &pNextCmdTargetFromView);

	if (SUCCEEDED(hr))
		pFilter->SetNextTarget(pNextCmdTargetFromView);

	return pFilter;
}

HRESULT Util_ReloadFile(const wchar_t* filePath)
{
	if (!filePath || filePath[0] == L'\0')
	{
		return E_INVALIDARG;
	}

	CComPtr<IServiceProvider> sp = g_ps.pServiceProvider;
	if (!sp)
	{
		return E_UNEXPECTED; // Service provider is essential
	}

	CComPtr<IVsRunningDocumentTable> pRDT;
	HRESULT hr = sp->QueryService(SID_SVsRunningDocumentTable, IID_IVsRunningDocumentTable, (void**)&pRDT);
	if (FAILED(hr) || !pRDT)
	{
		return hr; // Failed to get RDT
	}

	CComPtr<IVsHierarchy> pHier;
	VSITEMID itemid = VSITEMID_NIL;
	CComPtr<IUnknown> pDocDataUnk;
	VSCOOKIE docCookie = VSCOOKIE_NIL;

	// 尝试在RDT中查找文档
	// RDT_NoLock 表示我们只是查找，不请求编辑锁
	hr = pRDT->FindAndLockDocument(
		(DWORD)_VSRDTFLAGS::RDT_NoLock,
		filePath,
		&pHier,
		&itemid,
		&pDocDataUnk,
		&docCookie
	);

	if (FAILED(hr) || !pDocDataUnk)
	{
		// 文档未在RDT中打开，或者查找时出错。根据要求，我们什么也不做。
		// FindAndLockDocument 在失败时或 pDocDataUnk 为空时，通常返回 S_FALSE 或错误代码
		// 如果文件未找到，通常是 S_FALSE。我们将此视为"未打开，不执行任何操作"的情况。
		return S_FALSE; 
	}

	// 文档已打开，尝试获取 IVsPersistDocData
	CComPtr<IVsPersistDocData> pPersistDocData;
	hr = pDocDataUnk->QueryInterface(IID_IVsPersistDocData, (void**)&pPersistDocData);

	if (SUCCEEDED(hr) && pPersistDocData)
	{
		// 成功获取接口，重新加载文档数据
// 		hr = pPersistDocData->ReloadDocData(RDD_RemoveUndoStack);
		if (g_ps.pRDTEventsListener)
			g_ps.pRDTEventsListener->EnableReloadListen(false);

		hr = pPersistDocData->ReloadDocData(0);
		// 如果用户取消（例如，如果提示保存脏文档时），可能返回 OLE_E_PROMPTSAVECANCELLED

		if (g_ps.pRDTEventsListener)
			g_ps.pRDTEventsListener->EnableReloadListen(true);
	}
	else
	{
		// 无法获取 IVsPersistDocData 接口，这不应该发生在一个正常的文档对象上
		// 但如果发生了，我们认为无法重新加载
		hr = E_NOINTERFACE;
	}

	// pDocDataUnk 和 pHier 的 CComPtr 会自动处理 Release
	// 由于使用了 RDT_NoLock，通常不需要显式调用 UnlockDocument 来释放查找操作的锁，
	// 因为 docCookie 可能为 VSCOOKIE_NIL。即使非 NIL，RDT_NoLock 表示未持有写锁。

	return hr;
}

bool Util_IsFileDirty(const wchar_t* filePath)
{
	if (!filePath || filePath[0] == L'\0')
	{
		return false;
	}

	CComPtr<IServiceProvider> sp = g_ps.pServiceProvider;
	if (!sp)
		return false;

	CComPtr<IVsRunningDocumentTable> pRDT;
	HRESULT hr = sp->QueryService(SID_SVsRunningDocumentTable, IID_IVsRunningDocumentTable, (void**)&pRDT);
	if (FAILED(hr) || !pRDT)
		return false;

	CComPtr<IVsHierarchy> pHier;
	VSITEMID itemid = VSITEMID_NIL;
	CComPtr<IUnknown> pDocDataUnk;
	VSCOOKIE docCookie = VSCOOKIE_NIL;

	// 在RDT中查找文档
	hr = pRDT->FindAndLockDocument(
		(DWORD)_VSRDTFLAGS::RDT_NoLock,
		filePath,
		&pHier,
		&itemid,
		&pDocDataUnk,
		&docCookie
	);

	if (FAILED(hr) || docCookie == VSCOOKIE_NIL || !pDocDataUnk)
		return false;

	// 文档已打开，获取IVsPersistDocData接口
	CComPtr<IVsPersistDocData> pPersistDocData;
	hr = pDocDataUnk->QueryInterface(IID_IVsPersistDocData, (void**)&pPersistDocData);
	if (FAILED(hr) || !pPersistDocData)
		return false;

	// 检查文档是否脏（有未保存的修改）
	BOOL isDirty = FALSE;
	hr = pPersistDocData->IsDocDataDirty(&isDirty);
	if (FAILED(hr))
		return false;

	return isDirty?true:false;
}

bool Util_ExistFile(const char* filePath)
{
	if (!filePath || filePath[0] == '\0')
		return false;

	std::wstring wPath = utf8_to_widechar(filePath);
	WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
	return GetFileAttributesExW(wPath.c_str(), GetFileExInfoStandard, &fileAttrData) != FALSE;
}

bool Util_LoadFileContent(const std::wstring& wpath, std::vector<BYTE>& content)
{
	std::ifstream file;
	file.open(wpath.c_str(), std::ios::in | std::ios::binary);

	if (!file.is_open())
	{
		content.clear();
		return false;
	}

	// 获取文件大小
	file.seekg(0, std::ios::end);
	std::streampos size = file.tellg();
	file.seekg(0, std::ios::beg);

	// 如果文件为空，直接返回
	if (size == 0)
	{
		content.clear();
		file.close();
		return true;
	}

	// 读取整个文件到向量
	content.resize(static_cast<size_t>(size));
	if (!file.read(reinterpret_cast<char*>(content.data()), size))
	{
		content.clear();
		file.close();
		return false;
	}

	file.close();
	return true;
}

bool Util_SaveFileContent(const std::wstring& wpath, const std::vector<BYTE>& content)
{
	// 创建输出文件流
	std::ofstream file;
	file.open(wpath.c_str(), std::ios::out | std::ios::binary);

	if (!file.is_open())
	{
		return false;
	}

	// 直接写入二进制内容
	if (!content.empty())
	{
		file.write(reinterpret_cast<const char*>(content.data()), content.size());
	}

	// 检查写入是否成功
	if (file.fail())
	{
		file.close();
		return false;
	}

	file.close();
	return true;
}

FILETIME Util_GetZeroFileTime()
{
	FILETIME ft;
	memset(&ft, 0, sizeof(ft));
	return ft;
}

bool Util_EqualFileTime(const FILETIME& a, const FILETIME& b)
{
	return a.dwHighDateTime == b.dwHighDateTime && a.dwLowDateTime == b.dwLowDateTime;
}

FILETIME Util_GetFileTime(const std::wstring& wpath)
{
	WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
	if (GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &fileAttrData))
	{
		return fileAttrData.ftLastWriteTime;
	}
	return Util_GetZeroFileTime();
}

void Util_SetFileTime(const std::wstring& wpath, FILETIME& t)
{
	HANDLE hFile = CreateFileW(wpath.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		SetFileTime(hFile, NULL, NULL, &t);
		CloseHandle(hFile);
	}
}

std::wstring Util_ToLower(const std::wstring& str)
{
	std::wstring lowerStr = str;
	std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::towlower);
	return lowerStr;
}

int Util_GetFirstVisibleLine(CComPtr<IVsTextView> pVsTextView)
{
	if (!pVsTextView)
	{
		return -1;
	}

	long iMinUnit = 0;
	long iMaxUnit = 0;
	long iVisibleUnits = 0;
	long iFirstVisibleUnit = 0;

	// 获取垂直滚动条信息
	// SB_VERT = 1 (垂直滚动条)
	HRESULT hr = pVsTextView->GetScrollInfo(1, &iMinUnit, &iMaxUnit, &iVisibleUnits, &iFirstVisibleUnit);

	if (SUCCEEDED(hr))
	{
		// iFirstVisibleUnit 就是第一个可见单元（行）的索引
		// 注意：这个值是从0开始计数的
		return static_cast<int>(iFirstVisibleUnit);
	}

	return -1;
}

int Util_GetText(CComPtr<IVsTextView> pVsTextView, std::string& utf8Text)
{
	if (!pVsTextView)
	{
		return -1;
	}

	// 1. 获取文本缓冲区
	CComPtr<IVsTextLines> pTextLines;
	HRESULT hr = pVsTextView->GetBuffer(&pTextLines);
	if (FAILED(hr) || !pTextLines)
	{
		return -1;
	}

	// 2. 获取文本行数
	long lineCount = 0;
	hr = pTextLines->GetLineCount(&lineCount);
	if (FAILED(hr) || lineCount <= 0)
	{
		utf8Text.clear();
		return 0; // 空文档
	}

	// 3. 获取整个文本内容
	// 首先获取最后一行最后一个字符的位置
	long lastLineLength = 0;
	hr = pTextLines->GetLengthOfLine(lineCount - 1, &lastLineLength);
	if (FAILED(hr))
	{
		return -1;
	}

	// 4. 分配缓冲区
	// 计算需要的缓冲区大小（字符数）
	long totalChars = 0;
	for (long i = 0; i < lineCount; i++)
	{
		long lineLength = 0;
		if (SUCCEEDED(pTextLines->GetLengthOfLine(i, &lineLength)))
		{
			totalChars += lineLength + 1; // +1 用于换行符
		}
	}

	if (totalChars <= 0)
	{
		utf8Text.clear();
		return 0;
	}

	// 5. 获取文本内容
	std::wstring wstrContent;
	wstrContent.reserve(totalChars);

	for (long i = 0; i < lineCount; i++)
	{
		// 获取行文本
		long lineLength = 0;
		hr = pTextLines->GetLengthOfLine(i, &lineLength);
		if (FAILED(hr))
		{
			continue;
		}

		if (lineLength > 0)
		{
			// 分配缓冲区获取行文本
			BSTR bstrLine = nullptr;
			long actualLength = 0;
			hr = pTextLines->GetLineText(i, 0, i, lineLength, &bstrLine);
			if (SUCCEEDED(hr) && bstrLine)
			{
				wstrContent.append(bstrLine, SysStringLen(bstrLine));
				SysFreeString(bstrLine);
			}
		}

		// 添加换行符（除了最后一行）
		if (i < lineCount - 1)
		{
			wstrContent.push_back(L'\n');
		}
	}

	// 6. 转换为 UTF-8
	utf8Text = widechar_to_utf8(wstrContent.c_str());
	return static_cast<int>(utf8Text.length());
}

void Util_SetFirstVisibleLine(CComPtr<IVsTextView> pVsTextView, int line)
{
	if (!pVsTextView)
	{
		return;
	}

	// 确保行号非负
	if (line < 0)
	{
		line = 0;
	}

	// 方法1: 使用 SetTopLine 方法（如果可用）
	// 首先尝试使用 SetTopLine 方法
	HRESULT hr = pVsTextView->SetTopLine(line);
	if (SUCCEEDED(hr))
	{
		return; // 成功设置顶部行
	}

	// 方法3: 使用 EnsureSpanVisible 方法
	// 创建一个文本范围，确保目标行可见
	CComPtr<IVsTextLines> pTextLines;
	hr = pVsTextView->GetBuffer(&pTextLines);
	if (SUCCEEDED(hr) && pTextLines)
	{
		// 获取文档总行数
		long totalLines = 0;
		if (SUCCEEDED(pTextLines->GetLineCount(&totalLines)))
		{
			// 确保行号不超过文档总行数
			if (line >= totalLines)
			{
				line = totalLines - 1;
				if (line < 0) line = 0;
			}

			// 创建文本范围
			TextSpan ts;
			ts.iStartLine = line;
			ts.iStartIndex = 0;
			ts.iEndLine = line + 1; // 确保至少一行可见
			ts.iEndIndex = 0;

			// 确保范围可见
			pVsTextView->EnsureSpanVisible(ts);
		}
	}
}

// ==================== 断点/书签 保存与恢复（通过 C# COM 服务） ====================

// IBreakpointBookmarkService 的 COM 接口 GUID
// {A7B2C3D4-E5F6-4A7B-8C9D-0E1F2A3B4C5D}
static const GUID IID_IBreakpointBookmarkService =
{ 0xA7B2C3D4, 0xE5F6, 0x4A7B, { 0x8C, 0x9D, 0x0E, 0x1F, 0x2A, 0x3B, 0x4C, 0x5D } };

// SBreakpointBookmarkService 的 SID GUID
// {B8C9D0E1-F2A3-4B4C-5D6E-7F8A9B0C1D2E}
static const GUID SID_SBreakpointBookmarkService =
{ 0xB8C9D0E1, 0xF2A3, 0x4B4C, { 0x5D, 0x6E, 0x7F, 0x8A, 0x9B, 0x0C, 0x1D, 0x2E } };

// 获取 C# BreakpointBookmarkService 的 COM 接口
static CComPtr<IDispatch> Util_GetBreakpointBookmarkService()
{
	CComPtr<IServiceProvider> sp = g_ps.pServiceProvider;
	if (!sp)
		return nullptr;

	CComPtr<IUnknown> pUnk;
	if (FAILED(sp->QueryService(SID_SBreakpointBookmarkService, IID_IDispatch, (void**)&pUnk)) || !pUnk)
		return nullptr;

	CComPtr<IDispatch> pDisp;
	pUnk->QueryInterface(IID_IDispatch, (void**)&pDisp);
	return pDisp;
}

std::string Util_SaveBreakpointsForFile(const std::wstring& filePath)
{
	CComPtr<IDispatch> pService = Util_GetBreakpointBookmarkService();
	if (!pService)
		return {};

	// 调用 SaveBreakpoints(BSTR filePath) -> BSTR
	DISPID dispId = DISPID_UNKNOWN;
	OLECHAR* methodName = const_cast<OLECHAR*>(L"SaveBreakpoints");
	if (FAILED(pService->GetIDsOfNames(IID_NULL, &methodName, 1, LOCALE_USER_DEFAULT, &dispId)))
		return {};

	CComVariant arg(filePath.c_str());
	DISPPARAMS dp;
	dp.cArgs = 1;
	dp.rgvarg = &arg;
	dp.cNamedArgs = 0;
	dp.rgdispidNamedArgs = nullptr;

	CComVariant result;
	if (FAILED(pService->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
		DISPATCH_METHOD, &dp, &result, nullptr, nullptr)))
		return {};

	if (FAILED(result.ChangeType(VT_BSTR)))
		return {};

	return result.bstrVal ? std::string(_bstr_t(result.bstrVal)) : std::string();
}

std::string Util_SaveBookmarksForFile(const std::wstring& filePath)
{
	CComPtr<IDispatch> pService = Util_GetBreakpointBookmarkService();
	if (!pService)
		return {};

	DISPID dispId = DISPID_UNKNOWN;
	OLECHAR* methodName = const_cast<OLECHAR*>(L"SaveBookmarks");
	if (FAILED(pService->GetIDsOfNames(IID_NULL, &methodName, 1, LOCALE_USER_DEFAULT, &dispId)))
		return {};

	CComVariant arg(filePath.c_str());
	DISPPARAMS dp;
	dp.cArgs = 1;
	dp.rgvarg = &arg;
	dp.cNamedArgs = 0;
	dp.rgdispidNamedArgs = nullptr;

	CComVariant result;
	if (FAILED(pService->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
		DISPATCH_METHOD, &dp, &result, nullptr, nullptr)))
		return {};

	if (FAILED(result.ChangeType(VT_BSTR)))
		return {};

	return result.bstrVal ? std::string(_bstr_t(result.bstrVal)) : std::string();
}

// 辅助：将 lineMap (0-based old -> 0-based new) 转为字符串格式 "old:new;old:new;..."
static std::string LineMapToString(const std::vector<int>& lineMap)
{
	std::string result;
	for (int i = 0; i < (int)lineMap.size(); i++)
	{
		if (!result.empty())
			result += ';';
		result += std::to_string(i) + ':' + std::to_string(lineMap[i]);
	}
	return result;
}

void Util_RestoreBreakpoints(const std::wstring& filePath, const std::string& bpData, const std::vector<int>& lineMap)
{
	if (bpData.empty())
		return;

	CComPtr<IDispatch> pService = Util_GetBreakpointBookmarkService();
	if (!pService)
		return;

	DISPID dispId = DISPID_UNKNOWN;
	OLECHAR* methodName = const_cast<OLECHAR*>(L"RestoreBreakpoints");
	if (FAILED(pService->GetIDsOfNames(IID_NULL, &methodName, 1, LOCALE_USER_DEFAULT, &dispId)))
		return;

	std::string lineMapStr = LineMapToString(lineMap);
	std::wstring wLineMap(lineMapStr.begin(), lineMapStr.end());
	std::wstring wBpData(bpData.begin(), bpData.end());

	// 参数逆序: arg2=lineMapData, arg1=bpData, arg0=filePath
	CComVariant args[3];
	args[0] = wLineMap.c_str();
	args[1] = wBpData.c_str();
	args[2] = filePath.c_str();

	DISPPARAMS dp;
	dp.cArgs = 3;
	dp.rgvarg = args;
	dp.cNamedArgs = 0;
	dp.rgdispidNamedArgs = nullptr;

	CComVariant result;
	pService->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
		DISPATCH_METHOD, &dp, &result, nullptr, nullptr);
}

void Util_RestoreBookmarks(const std::wstring& filePath, const std::string& bmData, const std::vector<int>& lineMap)
{
	if (bmData.empty())
		return;

	CComPtr<IDispatch> pService = Util_GetBreakpointBookmarkService();
	if (!pService)
		return;

	DISPID dispId = DISPID_UNKNOWN;
	OLECHAR* methodName = const_cast<OLECHAR*>(L"RestoreBookmarks");
	if (FAILED(pService->GetIDsOfNames(IID_NULL, &methodName, 1, LOCALE_USER_DEFAULT, &dispId)))
		return;

	std::string lineMapStr = LineMapToString(lineMap);
	std::wstring wLineMap(lineMapStr.begin(), lineMapStr.end());
	std::wstring wBmData(bmData.begin(), bmData.end());

	// 参数逆序: arg2=lineMapData, arg1=bmData, arg0=filePath
	CComVariant args[3];
	args[0] = wLineMap.c_str();
	args[1] = wBmData.c_str();
	args[2] = filePath.c_str();

	DISPPARAMS dp;
	dp.cArgs = 3;
	dp.rgvarg = args;
	dp.cNamedArgs = 0;
	dp.rgdispidNamedArgs = nullptr;

	CComVariant result;
	pService->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
		DISPATCH_METHOD, &dp, &result, nullptr, nullptr);
}

