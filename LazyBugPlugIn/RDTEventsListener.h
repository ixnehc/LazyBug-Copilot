// Package.h

#pragma once
#include <atlbase.h>    // For CComObjectRootEx, CComCoClass, etc. (if making it a COM object)
#include <vsshell.h>    // For IVsRunningDocTableEvents3, VSCOOKIE, etc.
#include <vsshell80.h>  // For __VSDOCUMENTCHANGES, etc. (or later versions like vsshell160.h)
#include <oleidl.h>     // For IConnectionPointContainer (if needed, but Advise directly is simpler here)


class CRDTEventsListener :
	public CComObjectRootEx<CComSingleThreadModel>, // Or CComMultiThreadModel if appropriate
	public IVsRunningDocTableEvents3
	// If you need this object to be creatable via CoCreateInstance, add CComCoClass
{
public:
	CRDTEventsListener() : m_dwRdtCookie(0), m_pRDT(nullptr), m_disableReloadListen(false){}
	~CRDTEventsListener()
	{
		// Ensure Unadvise is called if not already
		Unadvise();
	}

	// This ATL macro is crucial for QueryInterface to work for IVsRunningDocTableEvents3
	BEGIN_COM_MAP(CRDTEventsListener)
		COM_INTERFACE_ENTRY(IVsRunningDocTableEvents3)
		COM_INTERFACE_ENTRY(IVsRunningDocTableEvents2)
		COM_INTERFACE_ENTRY(IVsRunningDocTableEvents)
	END_COM_MAP()

	bool HasAdvised()
	{
		return (m_dwRdtCookie != 0);
	}

	// Call this to register with RDT
	HRESULT Advise(IVsRunningDocumentTable* pRDT);

	// Call this to unregister from RDT
	HRESULT Unadvise();

	// --- IVsRunningDocTableEvents3 Methods ---

	STDMETHODIMP OnAfterFirstDocumentLock(VSCOOKIE docCookie, VSRDTFLAGS dwRDTLockType, DWORD dwReadLocksRemaining, DWORD dwEditLocksRemaining);
	STDMETHODIMP OnBeforeLastDocumentUnlock(VSCOOKIE docCookie, VSRDTFLAGS dwRDTLockType, DWORD dwReadLocksRemaining, DWORD dwEditLocksRemaining);

	STDMETHODIMP OnAfterSave(VSCOOKIE docCookie);

	STDMETHODIMP OnAfterAttributeChange(VSCOOKIE docCookie, VSRDTATTRIB grfAttribs)
	{
		return S_OK;
	}

	STDMETHODIMP OnBeforeDocumentWindowShow(VSCOOKIE docCookie, BOOL fFirstShow, IVsWindowFrame* pFrame)
	{
		return S_OK;
	}

	STDMETHODIMP OnAfterDocumentWindowHide(		VSCOOKIE docCookie,		IVsWindowFrame* pFrame)	
	{
		return S_OK;
	}

	STDMETHODIMP OnBeforeSave(VSCOOKIE docCookie);
	STDMETHODIMP OnAfterAttributeChangeEx(VSCOOKIE docCookie, VSRDTATTRIB grfAttribs, IVsHierarchy* pHierOld, VSITEMID itemidOld, LPCOLESTR pszMkDocumentOld, IVsHierarchy* pHierNew, VSITEMID itemidNew, LPCOLESTR pszMkDocumentNew);

	void EnableReloadListen(bool enable)	{		m_disableReloadListen = !enable;	}

private:
	DWORD m_dwRdtCookie;                        // Cookie for advising/unadvising
	IVsRunningDocumentTable* m_pRDT;            // Pointer to the RDT service

	bool m_disableReloadListen;
};