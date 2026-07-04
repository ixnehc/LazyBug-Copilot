#include "stdh.h"

#include "RDTEventsListener.h"
#include "PackageState.h"

#include "OpenedDocStates.h"
#include "Utils.h"

extern void RequestGenerateSlnDump();

	// Call this to register with RDT
HRESULT CRDTEventsListener::Advise(IVsRunningDocumentTable* pRDT)
{
	if (!pRDT) return E_POINTER;
	if (m_dwRdtCookie != 0) return S_FALSE; // Already advised

	m_pRDT = pRDT; // Store for Unadvise
	m_pRDT->AddRef(); // Hold a reference

	// The 'this' pointer is implicitly cast to IUnknown* and then QI'd by Advise for IVsRunningDocTableEvents3
	HRESULT hr=pRDT->AdviseRunningDocTableEvents(this, &m_dwRdtCookie);

	return hr;
}

	// Call this to unregister from RDT
HRESULT CRDTEventsListener::Unadvise()
{
	if (m_dwRdtCookie == 0 || !m_pRDT) return S_FALSE; // Not advised or RDT is null

	HRESULT hr = m_pRDT->UnadviseRunningDocTableEvents(m_dwRdtCookie);
	m_dwRdtCookie = 0;
	if (m_pRDT)
	{
		m_pRDT->Release();
		m_pRDT = nullptr;
	}
	return hr;
}

STDMETHODIMP CRDTEventsListener::OnBeforeSave(VSCOOKIE docCookie)
{
	// OutputDebugString(L"CRDTEventsListener::OnBeforeSave - Attempting to block save.\n");

	// 获取文档信息，例如文件名，以决定是否阻止
	// CComPtr<IVsRunningDocumentTable> pRDT; // You already have m_pRDT
	// if (m_pRDT)
	// {
	//     DWORD dwRDTFlags, dwReadLocks, dwEditLocks;
	//     BSTR bstrMkDocument = nullptr;
	//     CComPtr<IVsHierarchy> pHier;
	//     VSITEMID itemid;
	//     IUnknown* pDocData = nullptr;
	//     HRESULT hrInfo = m_pRDT->GetDocumentInfo(docCookie, &dwRDTFlags, &dwReadLocks, &dwEditLocks, &bstrMkDocument, &pHier, &itemid, &pDocData);
	//     if (SUCCEEDED(hrInfo))
	//     {
	//         if (bstrMkDocument)
	//         {
	//             // OLECHAR* pszPath = bstrMkDocument;
	//             // OutputDebugStringW(L"Attempting to save: ");
	//             // OutputDebugStringW(pszPath);
	//             // OutputDebugStringW(L"\n");
	//             // TODO: Add logic here to decide if this specific document's save should be blocked.
	//             // For example, if (wcscmp(pszPath, L"C:\\path\\to\\specific\\file.txt") == 0) { ... }
	//             SysFreeString(bstrMkDocument);
	//         }
	//         if (pDocData) pDocData->Release();
	//     }
	// }


	// **要阻止保存，返回一个失败的 HRESULT，例如 E_ABORT。**
	// S_FALSE 通常表示“取消操作”，在某些上下文中也可能起作用，但 E_ABORT 更明确。
	// 你也可以返回自定义的错误码，但VS可能不会以你期望的方式处理它。
	//
	// MessageBox(NULL, L"Saving this document is currently blocked by MyExtension.", L"Save Blocked", MB_OK | MB_ICONWARNING);
	return S_OK; // 允许保存操作，不阻止
}


STDMETHODIMP CRDTEventsListener::OnAfterFirstDocumentLock(VSCOOKIE docCookie, VSRDTFLAGS dwRDTLockType, DWORD dwReadLocksRemaining, DWORD dwEditLocksRemaining)
{
	// 当文档首次被锁定时（通常是打开时），记录文件状态
	if (m_pRDT)
	{
		BSTR bstrMkDocument = nullptr;
		CComPtr<IVsHierarchy> pHier;
		VSITEMID itemid;
		DWORD dwRDTFlags, dwReadLocks, dwEditLocks;
		IUnknown* pDocData = nullptr;
		
		HRESULT hr = m_pRDT->GetDocumentInfo(docCookie, &dwRDTFlags, &dwReadLocks, &dwEditLocks, 
			&bstrMkDocument, &pHier, &itemid, &pDocData);
		
		if (SUCCEEDED(hr) && bstrMkDocument)
		{
			std::wstring filePath(bstrMkDocument);
			FILETIME modifiedTime = Util_GetFileTime(filePath);
			g_openDocStates.Add(filePath);
			g_openDocStates.Update(filePath); // 这会更新修改时间
			
			SysFreeString(bstrMkDocument);
		}
		
		if (pDocData)
			pDocData->Release();
	}
	
	return S_OK;
}
STDMETHODIMP CRDTEventsListener::OnBeforeLastDocumentUnlock(VSCOOKIE docCookie, VSRDTFLAGS dwRDTLockType, DWORD dwReadLocksRemaining, DWORD dwEditLocksRemaining)
{
	// 当文档最后一个锁被释放时（通常是关闭时），移除文件状态
	if (m_pRDT)
	{
		BSTR bstrMkDocument = nullptr;
		CComPtr<IVsHierarchy> pHier;
		VSITEMID itemid;
		DWORD dwRDTFlags, dwReadLocks, dwEditLocks;
		IUnknown* pDocData = nullptr;
		
		HRESULT hr = m_pRDT->GetDocumentInfo(docCookie, &dwRDTFlags, &dwReadLocks, &dwEditLocks, 
			&bstrMkDocument, &pHier, &itemid, &pDocData);
		
		if (SUCCEEDED(hr) && bstrMkDocument)
		{
			std::wstring filePath(bstrMkDocument);
			g_openDocStates.Remove(filePath);
			
			SysFreeString(bstrMkDocument);
		}
		
		if (pDocData)
			pDocData->Release();
	}
	
	return S_OK;
}

STDMETHODIMP CRDTEventsListener::OnAfterSave(VSCOOKIE docCookie)
{
	if (!m_pRDT || !g_ps.pServiceProvider) return S_OK;

	ULONG flags, readLocks, editLocks;
	CComBSTR bstrMkDocument;
	CComPtr<IVsHierarchy> pHier;
	VSITEMID itemid;
	CComPtr<IUnknown> pDocData;

	// 获取被保存文档的信息
	HRESULT hr = m_pRDT->GetDocumentInfo(
		docCookie, &flags, &readLocks, &editLocks,
		&bstrMkDocument, &pHier, &itemid, &pDocData);

	if (SUCCEEDED(hr) && bstrMkDocument.m_str)
	{
		std::wstring filePath(bstrMkDocument.m_str);
		g_openDocStates.Update(filePath); // 这会更新修改时间

// 		// 显示保存的文档名（调试用）
// 		MessageBoxW(NULL, filePath.c_str(), L"OnAfterSave - 文档被保存", MB_OK);
	} 

	if (FAILED(hr)) return S_OK;

	// ==========================================
	// 1. 判断是否是 Project 或者 Solution 文件被保存
	//    通过 VSRDTFLAGS 的 RDT_ProjSlnDocument 标志可以可靠地判断文件是否为项目或解决方案文件
	//    同时增加文件后缀的后备判断，增强可靠性
	// ==========================================
	bool isProjSln = (flags & _VSRDTFLAGS::RDT_ProjSlnDocument) != 0;

	if (!isProjSln && bstrMkDocument.m_str)
	{
		std::wstring filePath(bstrMkDocument.m_str);
		size_t dotPos = filePath.find_last_of(L'.');
		if (dotPos != std::wstring::npos)
		{
			std::wstring ext = filePath.substr(dotPos);
			std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
			if (ext == L".sln" || ext == L".vcxproj" || ext == L".filters" || ext == L".vcxitems" || 
				ext == L".csproj" || ext == L".vbproj" || ext == L".fsproj")
			{
				isProjSln = true;
			}
		}
	}

	if (isProjSln)
	{
		RequestGenerateSlnDump();

		// TODO: 此处添加针对 Project/Solution 文件保存后的处理逻辑
		return S_OK;
	}

	// ==========================================
	// 3. 普通文档文件被保存
	// ==========================================
// 	MessageBoxA(NULL, "bb", "BB", MB_OK);

	return S_OK;
}

STDMETHODIMP CRDTEventsListener::OnAfterAttributeChangeEx(VSCOOKIE docCookie, VSRDTATTRIB grfAttribs, IVsHierarchy* pHierOld, VSITEMID itemidOld, LPCOLESTR pszMkDocumentOld, IVsHierarchy* pHierNew, VSITEMID itemidNew, LPCOLESTR pszMkDocumentNew)
{
	if (grfAttribs & RDTA_DocDataReloaded)
	{
		if (!m_disableReloadListen)
		{
			// 当文档重新加载时，更新文件状态中的修改时间
			if (m_pRDT)
			{
				BSTR bstrMkDocument = nullptr;
				CComPtr<IVsHierarchy> pHier;
				VSITEMID itemid;
				DWORD dwRDTFlags, dwReadLocks, dwEditLocks;
				IUnknown* pDocData = nullptr;

				HRESULT hr = m_pRDT->GetDocumentInfo(docCookie, &dwRDTFlags, &dwReadLocks, &dwEditLocks,
					&bstrMkDocument, &pHier, &itemid, &pDocData);

				if (SUCCEEDED(hr) && bstrMkDocument)
				{
					std::wstring filePath(bstrMkDocument);
					g_openDocStates.Update(filePath); // 更新文件状态中的修改时间

					SysFreeString(bstrMkDocument);
				}

				if (pDocData)
					pDocData->Release();
			}
		}
	}
	
	return S_OK;
}
