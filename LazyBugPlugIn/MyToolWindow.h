//
// MyToolWindow.h
//
// This file contains the implementation of a tool window that hosts a .NET user control
//

#pragma once

using namespace VSL;
#include <AtlWin.h>
#include <VSLWindows.h>
#include <vsshell.h>

#include "..\LazyBugPlugInUI\Resource.h"

#include "../LazyBugPlugInControls/LazyBugPlugInControlsExport.h"

#include "../LazyBugSource/SolutionDump.h"

#include "Utils.h"


// {624ed9c3-bdfd-41fa-96c3-7c824ea32e3d}
DEFINE_GUID(EnvironmentColorsCategory, 
0x624ed9c3, 0xbdfd, 0x41fa, 0x96, 0xc3, 0x7c, 0x82, 0x4e, 0xa3, 0x2e, 0x3d);

#define VS_RGBA_TO_COLORREF(rgba) (rgba & 0x00FFFFFF)

#define IDT_CHATDIALOG_TIMER 1 // 定时器ID

#define WM_FORCE_UPDATE_FILECHANGE_ATTACH (WM_USER+100) //要与ChatDialog.cpp里的WM_FORCE_UPDATE_FILECHANGE_ATTACH保持一致

class LazyBugPlugInWindowPane :
	public CComObjectRootEx<CComSingleThreadModel>,
	public VsWindowPaneFromResource<LazyBugPlugInWindowPane, IDD_LazyBugPlugIn_DLG>,
	public VsWindowFrameEventSink<LazyBugPlugInWindowPane>,
	public VSL::ISupportErrorInfoImpl<
		InterfaceSupportsErrorInfoList<IVsWindowPane,
		InterfaceSupportsErrorInfoList<IVsWindowFrameNotify,
		InterfaceSupportsErrorInfoList<IVsWindowFrameNotify3> > > >,
		public IVsBroadcastMessageEvents
{
	VSL_DECLARE_NOT_COPYABLE(LazyBugPlugInWindowPane)

protected:

	// Protected constructor called by CComObject<LazyBugPlugInWindowPane>::CreateInstance.
	LazyBugPlugInWindowPane() :
		VsWindowPaneFromResource(),
		m_hBackground(nullptr),
		m_BroadcastCookie(VSCOOKIE_NIL),
		m_SolutionCookie(VSCOOKIE_NIL),
		m_hChatDialog(nullptr)
	{}

	~LazyBugPlugInWindowPane() {}
	
public:

	BEGIN_COM_MAP(LazyBugPlugInWindowPane)
		COM_INTERFACE_ENTRY(IVsWindowPane)
		COM_INTERFACE_ENTRY(IVsWindowFrameNotify)
		COM_INTERFACE_ENTRY(IVsWindowFrameNotify3)
		COM_INTERFACE_ENTRY(ISupportErrorInfo)
		COM_INTERFACE_ENTRY(IVsBroadcastMessageEvents)
	END_COM_MAP()

	BEGIN_MSG_MAP(LazyBugPlugInWindowPane)

		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog) 
		COMMAND_HANDLER(IDC_CLICKME_BTN, BN_CLICKED, OnButtonClick)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_TIMER, OnTimer) // 添加 WM_TIMER 处理
		MESSAGE_HANDLER(WM_FORCE_UPDATE_FILECHANGE_ATTACH, OnForceUpdateFileChangeAttach) // 处理自定义消息

	END_MSG_MAP()


	// 在 MyToolWindow.cpp (或者如果实现在 .h 中，则在 .h 中) 添加实现:
	STDMETHODIMP LazyBugPlugInWindowPane::TranslateAccelerator(LPMSG lpMsg) override;

	// Function called by VsWindowPaneFromResource at the end of SetSite; at this point the
	// window pane is constructed and sited and can be used, so this is where we can initialize
	// the event sink by siting it.
	void PostSited(IVsPackageEnums::SetSiteResult /*result*/);

	// Function called by VsWindowPaneFromResource at the end of ClosePane.
	// Perform necessary cleanup.
	void PostClosed();

		// Callback function called by ToolWindowBase when the size of the window changes.
	void OnFrameSize(int x, int y, int w, int h);

	LRESULT OnButtonClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled);

	// Handled to set the color that should be used to draw the background of the Window Pane.
	LRESULT OnCtlColorDlg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); // 声明 OnTimer 方法

	STDMETHOD(OnBroadcastMessage)(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnForceUpdateFileChangeAttach(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

// 
// 	// IVsSolutionEvents 接口实现
// 	STDMETHOD(OnAfterOpenSolution)(IUnknown* pUnkReserved, BOOL fNewSolution);
// 	STDMETHOD(OnAfterCloseSolution)(IUnknown* pUnkReserved);
// 	STDMETHOD(OnAfterOpenProject)(IVsHierarchy* pHierarchy, BOOL fAdded);
// 	STDMETHOD(OnQueryCloseProject)(IVsHierarchy* pHierarchy, BOOL fRemoving, BOOL* pfCancel);
// 	STDMETHOD(OnBeforeCloseProject)(IVsHierarchy* pHierarchy, BOOL fRemoved);
// 	STDMETHOD(OnAfterLoadProject)(IVsHierarchy* pStubHierarchy, IVsHierarchy* pRealHierarchy);
// 	STDMETHOD(OnQueryUnloadProject)(IVsHierarchy* pRealHierarchy, BOOL* pfCancel);
// 	STDMETHOD(OnBeforeUnloadProject)(IVsHierarchy* pRealHierarchy, IVsHierarchy* pStubHierarchy);
// //	STDMETHOD(OnAfterOpenSolution)(IUnknown* pUnkReserved);
// 	STDMETHOD(OnAfterMergeSolution)(IUnknown* pUnkReserved);
// 	STDMETHOD(OnBeforeCloseSolution)(IUnknown* pUnkReserved);
// 	STDMETHOD(OnAfterRenameProject)(IVsHierarchy* pHierarchy);
// 	STDMETHOD(OnQueryCloseSolution)(IUnknown* pUnkReserved, BOOL* pfCancel);
// 	STDMETHOD(OnAfterAsynchOpenProject)(IVsHierarchy* pHierarchy, BOOL fAdded);

private:
	// Initialize colors that are used to render the Window Pane.
	void InitVSColors();

	void UpdateChatInputEscape();
	void EscapeChatInput();
	void UpdateSolutionDump();
	void UpdateEventListener();

	HBRUSH m_hBackground;
	VSCOOKIE m_BroadcastCookie;
	VSCOOKIE m_SolutionCookie;
	HWND m_hChatDialog;
	GenerateSlnDumpProgress m_GenerateSlnDumpProgress;
	SolutionDump m_slnDmp;
};


class LazyBugPlugInToolWindow :
	public VSL::ToolWindowBase<LazyBugPlugInToolWindow>
{
public:
	// Constructor of the tool window object.
	// The goal of this constructor is to initialize the base class with the site cache
	// of the owner package.
	LazyBugPlugInToolWindow(const PackageVsSiteCache& rPackageVsSiteCache):
		ToolWindowBase(rPackageVsSiteCache)
	{
	}

	// Caption of the tool window.
	const wchar_t* const GetCaption() const
	{
		static CStringW strCaption;
		// Avoid to load the string from the resources more that once.
		if (0 == strCaption.GetLength())
		{
			VSL_CHECKBOOL_GLE(
				strCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_WINDOW_TITLE));
		}
		return strCaption;
	}

	// Creation flags for this tool window.
	VSCREATETOOLWIN GetCreationFlags() const
	{
		return CTW_fInitNew|CTW_fForceCreate;
	}

	// Return the GUID of the persistence slot for this tool window.
	const GUID& GetToolWindowGuid() const
	{
		return CLSID_guidPersistanceSlot;
	}

	IUnknown* GetViewObject()
	{
		// Should only be called once per-instance
		VSL_CHECKBOOLEAN_EX(m_spView == NULL, E_UNEXPECTED, IDS_E_GETVIEWOBJECT_CALLED_AGAIN);

		// Create the object that implements the window pane for this tool window.
		CComObject<LazyBugPlugInWindowPane>* pViewObject;
		VSL_CHECKHRESULT(CComObject<LazyBugPlugInWindowPane>::CreateInstance(&pViewObject));

		// Get the pointer to IUnknown for the window pane.
		HRESULT hr = pViewObject->QueryInterface(IID_IUnknown, (void**)&m_spView);
		if (FAILED(hr))
		{
			// If QueryInterface failed, then there is something wrong with the object.
			// Delete it and throw an exception for the error.
			delete pViewObject;
			VSL_CHECKHRESULT(hr);
		}

		return m_spView;
	}

	// This method is called by the base class after the tool window is created.
	// We use it to set the icon for this window.
	void PostCreate()
	{
		CComVariant srpvt;
		srpvt.vt = VT_I4;
		srpvt.intVal = IDB_IMAGES;
		// We don't want to make the window creation fail only becuase we can not set
		// the icon, so we will not throw if SetProperty fails.
		if (SUCCEEDED(GetIVsWindowFrame()->SetProperty(VSFPROPID_BitmapResource, srpvt)))
		{
			srpvt.intVal = 1;
			GetIVsWindowFrame()->SetProperty(VSFPROPID_BitmapIndex, srpvt);
		}
	}
private:
	CComPtr<IUnknown> m_spView;

};
