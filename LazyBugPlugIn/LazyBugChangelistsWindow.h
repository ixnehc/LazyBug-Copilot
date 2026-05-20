//
// LazyBugChangelistsWindow.h
//
// This file contains the implementation of a new tool window
//

#pragma once

using namespace VSL;

#include <AtlWin.h>
#include <VSLWindows.h>

// TODO: You might need a new resource header if your new dialog is in a different resource DLL
// For now, we assume it might use existing resources or a new ID in the same one.
#include "..\LazyBugPlugInUI\Resource.h" 

#include "../LazyBugPlugInControls/LazyBugPlugInControlsExport.h"

#include "FileChangeAttach.h"


#define VS_RGBA_TO_COLORREF(rgba) (rgba & 0x00FFFFFF)
#define IDT_CHANGELISTS_TIMER 1 // 定时器ID

// Forward declaration
class LazyBugChangelistsWindow; 

class LazyBugChangelistsWindowPane :
	public CComObjectRootEx<CComSingleThreadModel>,
	// Replace IDD_LazyBugPlugIn_DLG with your new Dialog ID (e.g., IDD_LazyBugChangelistsWindow_DLG)
	public VsWindowPaneFromResource<LazyBugChangelistsWindowPane, IDD_LazyBugChangelistsWindow_DLG>,
	public VsWindowFrameEventSink<LazyBugChangelistsWindowPane>,
	public VSL::ISupportErrorInfoImpl<
		InterfaceSupportsErrorInfoList<IVsWindowPane,
		InterfaceSupportsErrorInfoList<IVsWindowFrameNotify,
		InterfaceSupportsErrorInfoList<IVsWindowFrameNotify3> > > >,
	public IVsBroadcastMessageEvents
{
	VSL_DECLARE_NOT_COPYABLE(LazyBugChangelistsWindowPane)

protected:
	LazyBugChangelistsWindowPane() :
		VsWindowPaneFromResource(),
		m_hBackground(nullptr),
		m_BroadcastCookie(VSCOOKIE_NIL),
		// m_SolutionCookie might not be needed unless this window also specifically tracks solution events
		m_hContentDialog(nullptr) // Example: if you host another dialog or custom HWND
	{}

	~LazyBugChangelistsWindowPane() {}
	
public:
	BEGIN_COM_MAP(LazyBugChangelistsWindowPane)
		COM_INTERFACE_ENTRY(IVsWindowPane)
		COM_INTERFACE_ENTRY(IVsWindowFrameNotify)
		COM_INTERFACE_ENTRY(IVsWindowFrameNotify3)
		COM_INTERFACE_ENTRY(ISupportErrorInfo)
		COM_INTERFACE_ENTRY(IVsBroadcastMessageEvents)
	END_COM_MAP()

	BEGIN_MSG_MAP(LazyBugChangelistsWindowPane)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		// TODO: Add message handlers for your new dialog's controls and messages
		// COMMAND_HANDLER(IDC_MY_NEW_BUTTON, BN_CLICKED, OnMyNewButtonClick)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_TIMER, OnTimer) // 添加 WM_TIMER 处理
	END_MSG_MAP()

	STDMETHODIMP TranslateAccelerator(LPMSG lpMsg) override;

	void PostSited(IVsPackageEnums::SetSiteResult result);
	void PostClosed();
	void OnFrameSize(int x, int y, int w, int h);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); // 声明 OnTimer 方法
	// TODO: Add handlers for your specific controls if any
	// LRESULT OnMyNewButtonClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled);


	STDMETHOD(OnBroadcastMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	void InitVSColors();
	//HWND CreateMyContentDialog(HWND hParent); // Example if you create a sub-dialog

	HBRUSH m_hBackground;
	VSCOOKIE m_BroadcastCookie;
	HWND m_hContentDialog; // Example: HWND for your main content (e.g. a child dialog)

};


class LazyBugChangelistsWindow :
	public VSL::ToolWindowBase<LazyBugChangelistsWindow>
{
public:
	LazyBugChangelistsWindow(const PackageVsSiteCache& rPackageVsSiteCache):
		ToolWindowBase(rPackageVsSiteCache)
	{
	}

	const wchar_t* const GetCaption() const
	{
		static CStringW strCaption;
		if (0 == strCaption.GetLength())
		{
			UINT titleResourceID = IDS_CHANGELISTS_TITLE; 
			VSL_CHECKBOOL_GLE(
				strCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), titleResourceID));
		}
		return strCaption;
	}

	VSCREATETOOLWIN GetCreationFlags() const
	{
		return CTW_fInitNew|CTW_fForceCreate;
	}

	const GUID& GetToolWindowGuid() const
	{
		// This should be the new GUID you defined in guids.h
		return CLSID_guidLazyBugChangelistsWindowPersistanceSlot; 
	}

	IUnknown* GetViewObject()
	{
		VSL_CHECKBOOLEAN_EX(m_spView == NULL, E_UNEXPECTED, IDS_E_GETVIEWOBJECT_CALLED_AGAIN); // Assuming IDS_E_GETVIEWOBJECT_CALLED_AGAIN is a general error string

		CComObject<LazyBugChangelistsWindowPane>* pViewObject;
		VSL_CHECKHRESULT(CComObject<LazyBugChangelistsWindowPane>::CreateInstance(&pViewObject));

		HRESULT hr = pViewObject->QueryInterface(IID_IUnknown, (void**)&m_spView);
		if (FAILED(hr))
		{
			delete pViewObject;
			VSL_CHECKHRESULT(hr);
		}
		return m_spView;
	}

	void PostCreate()
	{
		// TODO: If you have a specific icon for this new tool window, set it here.
		// You might need a new bitmap resource ID (e.g. IDB_MYNEWTOOLWINDOW_IMAGES)
		// For now, this is similar to the original or can be adapted.
		// UINT iconResourceID = IDB_IMAGES; // REPLACE THIS if needed
		// int iconIndex = 1; // REPLACE THIS if needed

		/* CComVariant srpvt;
		srpvt.vt = VT_I4;
		srpvt.intVal = iconResourceID; 
		if (SUCCEEDED(GetIVsWindowFrame()->SetProperty(VSFPROPID_BitmapResource, srpvt)))
		{
			srpvt.intVal = iconIndex;
			GetIVsWindowFrame()->SetProperty(VSFPROPID_BitmapIndex, srpvt);
		}
		*/
	}

private:

	CComPtr<IUnknown> m_spView;
};

// Function to create the actual content (e.g., a dialog) hosted in LazyBugChangelistsWindowPane
// This is just an example placeholder. You'll need to implement this based on your UI.
// HWND CreateMyContentDialog(HWND hParent)
// {
//    // Example: return CreateDialogParam(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDD_YOUR_CONTENT_DIALOG), hParent, YourDialogProc, 0);
//    return NULL; 
// }
