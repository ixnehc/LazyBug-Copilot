// Package.h

#pragma once

#include <atlstr.h>
#include <VSLCommandTarget.h>

#include "resource.h"       // main symbols
#include "Guids.h"
#include "..\LazyBugPlugInUI\Resource.h"

#include "..\LazyBugPlugInUI\CommandIds.h"

#include "../LazyBugPlugInControls/LazyBugPlugInControlsExport.h"

#include <initguid.h>
#include "guids.h"

#include "MyToolWindow.h"
#include "LazyBugChangelistsWindow.h"
#include "RDTEventsListener.h"
#include <commctrl.h>
#include <vsshell.h> // For IVsSolutionEvents etc.

#include "PackageState.h"


using namespace VSL;

extern void RequestGenerateSlnDump();


class ATL_NO_VTABLE CLazyBugPlugInPackage :
	// CComObjectRootEx and CComCoClass are used to implement a non-thread safe COM object, and 
	// a partial implementation for IUnknown (the COM map below provides the rest).
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLazyBugPlugInPackage, &CLSID_LazyBugPlugIn>,
	// Provides the implementation for IVsPackage to make this COM object into a VS Package.
	public IVsPackageImpl<CLazyBugPlugInPackage, &CLSID_LazyBugPlugIn>,
	public IOleCommandTargetImpl<CLazyBugPlugInPackage>,
	// Provides consumers of this object with the ability to determine which interfaces support
	// extended error information.
	public ATL::ISupportErrorInfoImpl<&__uuidof(IVsPackage)>,
	public IVsSolutionEvents
{
public:

	// Provides a portion of the implementation of IUnknown, in particular the list of interfaces
	// the CLazyBugPlugInPackage object will support via QueryInterface
	BEGIN_COM_MAP(CLazyBugPlugInPackage)
		COM_INTERFACE_ENTRY(IVsPackage)
		COM_INTERFACE_ENTRY(IOleCommandTarget)
		COM_INTERFACE_ENTRY(ISupportErrorInfo)
		COM_INTERFACE_ENTRY(IVsSolutionEvents)
	END_COM_MAP()

	// COM objects typically should not be cloned, and this prevents cloning by declaring the 
	// copy constructor and assignment operator private (NOTE:  this macro includes the declaration of
	// a private section, so everything following this macro and preceding a public or protected 
	// section will be private).
	VSL_DECLARE_NOT_COPYABLE(CLazyBugPlugInPackage)

public:
	CLazyBugPlugInPackage() :

		m_MyToolWindow(GetVsSiteCache()),
		m_changeslistsWindow(GetVsSiteCache())
	{
	}

	~CLazyBugPlugInPackage()
	{
	}
	
	// IVsPackage 实现重写
	STDMETHOD(SetSite)(IServiceProvider* pServiceProvider)
	{
		HRESULT hr = IVsPackageImpl<CLazyBugPlugInPackage, &CLSID_LazyBugPlugIn>::SetSite(pServiceProvider);
		if (pServiceProvider)
		{
			if (SUCCEEDED(hr))
				_InitState(pServiceProvider);
		}

		RequestGenerateSlnDump();
		_CheckAndOpenSolution();
		return hr;
	}

	// 在包关闭时取消订阅事件
	STDMETHOD(Close)()
	{
		_ClearState();
		return IVsPackageImpl<CLazyBugPlugInPackage, &CLSID_LazyBugPlugIn>::Close();
	}

	// IVsSolutionEvents 方法实现
	STDMETHOD(OnAfterOpenProject)(IVsHierarchy* pHierarchy, BOOL fAdded) 
	{ 
		return S_OK; 
	}
	STDMETHOD(OnQueryCloseProject)(IVsHierarchy* pHierarchy, BOOL fRemoving, BOOL* pfCancel) { return S_OK; }
	STDMETHOD(OnBeforeCloseProject)(IVsHierarchy* pHierarchy, BOOL fRemoved) { return S_OK; }
	STDMETHOD(OnAfterLoadProject)(IVsHierarchy* pStubHierarchy, IVsHierarchy* pRealHierarchy) 
	{ 
		RequestGenerateSlnDump();
		return S_OK;
	}
	STDMETHOD(OnQueryUnloadProject)(IVsHierarchy* pRealHierarchy, BOOL* pfCancel) { return S_OK; }
	STDMETHOD(OnBeforeUnloadProject)(IVsHierarchy* pRealHierarchy, IVsHierarchy* pStubHierarchy) { return S_OK; }

	STDMETHOD(OnAfterOpenSolution)(IUnknown* pUnkReserved, BOOL fNewSolution)
	{
		RequestGenerateSlnDump();
		_CheckAndOpenSolution();
		return S_OK;
	}

	STDMETHOD(OnBeforeCloseSolution)(IUnknown* pUnkReserved)
	{
		if (g_ps.pRDTEventsListener)
			g_ps.pRDTEventsListener->Unadvise();
		_CloseSolution();
		return S_OK;
	}


	STDMETHOD(OnQueryCloseSolution)(IUnknown* pUnkReserved, BOOL* pfCancel) { return S_OK; }
	STDMETHOD(OnAfterCloseSolution)(IUnknown* pUnkReserved) { return S_OK; }
	STDMETHOD(OnAfterMergeSolution)(IUnknown* pUnkReserved) { return S_OK; }
	STDMETHOD(OnAfterRenameProject)(IVsHierarchy* pHierarchy) { return S_OK; }
	STDMETHOD(OnAfterAsynchOpenProject)(IVsHierarchy* pHierarchy, BOOL fAdded) { return S_OK; }

	// This function provides the error information if it is not possible to load
	// the UI dll. It is for this reason that the resource IDS_E_BADINSTALL must
	// be defined inside this dll's resources.
	static const LoadUILibrary::ExtendedErrorInfo& GetLoadUILibraryErrorInfo()
	{
		static LoadUILibrary::ExtendedErrorInfo errorInfo(IDS_E_BADINSTALL);
		return errorInfo;
	}

	// DLL is registered with VS via a pkgdef file. Don't do anything if asked to
	// self-register.
	static HRESULT WINAPI UpdateRegistry(BOOL bRegister)
	{
		return S_OK;
	}

	// NOTE - the arguments passed to these macros can not have names longer then 30 characters
	// Definition of the commands handled by this package
	VSL_BEGIN_COMMAND_MAP()

		VSL_COMMAND_MAP_ENTRY(CLSID_LazyBugPlugInCmdSet, LazyBugChat, NULL, CommandHandler::ExecHandler(&OnMyTool))
// 		VSL_COMMAND_MAP_ENTRY(CLSID_LazyBugPlugInCmdSet, LazyBugChangelists, NULL, CommandHandler::ExecHandler(&OnChangelists))
		VSL_COMMAND_MAP_ENTRY(CLSID_LazyBugPlugInCmdSet, LazyBugAddFileRef, NULL, CommandHandler::ExecHandler(&OnLazyBugAddFileRef))
		VSL_COMMAND_MAP_ENTRY(CLSID_LazyBugPlugInCmdSet, LazyBugAddTabFileRef, NULL, CommandHandler::ExecHandler(&OnLazyBugAddTabFileRef))
		VSL_END_VSCOMMAND_MAP()


		// The tool map implements IVsPackage::CreateTool that is called by VS to create a tool window 
		// when appropriate.
		VSL_BEGIN_TOOL_MAP()
		VSL_TOOL_ENTRY(CLSID_guidPersistanceSlot, m_MyToolWindow.CreateAndShow())
// 		VSL_TOOL_ENTRY(CLSID_guidLazyBugChangelistsWindowPersistanceSlot, m_changeslistsWindow.CreateAndShow())
		VSL_END_TOOL_MAP()

	// Command handler called when the user selects the command to show the toolwindow.
	void OnMyTool(CommandHandler* /*pSender*/, DWORD /*flags*/, VARIANT* /*pIn*/, VARIANT* /*pOut*/)
	{
		m_MyToolWindow.CreateAndShow();
		extern void SetFocusToChatInput();
		SetFocusToChatInput();
	}

	// Command handler for the new tool window
	// TODO: Uncomment and implement if you have a command to open it
	void OnChangelists(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
	{
		m_changeslistsWindow.CreateAndShow();
	}

	// Command handler for LazyBug Add File Ref
	void OnLazyBugAddTabFileRef(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
	{
		m_MyToolWindow.CreateAndShow();

		extern PackageState g_ps;
		if (!g_ps.pTextManager) return;

		CComPtr<IVsTextView> pView;
		// GetActiveView(FALSE, ...) returns the last active view if the current window is not a text view.
		if (SUCCEEDED(g_ps.pTextManager->GetActiveView(FALSE, NULL, &pView)) && pView)
		{
			CComPtr<IVsTextLines> pBuffer;
			if (SUCCEEDED(pView->GetBuffer(&pBuffer)) && pBuffer)
			{
				CComQIPtr<IPersistFileFormat> pPersistFileFormat(pBuffer);
				if (pPersistFileFormat)
				{
					LPOLESTR pszFilename = NULL;
					DWORD formatIndex = 0;
					if (SUCCEEDED(pPersistFileFormat->GetCurFile(&pszFilename, &formatIndex)) && pszFilename)
					{
						CString strPath(pszFilename);
						::CoTaskMemFree(pszFilename);

						extern void AddFileToChat(const wchar_t* fullPath);
						AddFileToChat(strPath);
					}
				}
			}
		}
	}

	// Command handler for LazyBug Add File Ref
	void OnLazyBugAddFileRef(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
	{
		m_MyToolWindow.CreateAndShow();

		extern PackageState g_ps;
		if (!g_ps.pServiceProvider) return;

		CComPtr<IVsMonitorSelection> pMonitorSelection;
		if (SUCCEEDED(g_ps.pServiceProvider->QueryService(SID_SVsShellMonitorSelection, IID_IVsMonitorSelection, (void**)&pMonitorSelection)) && pMonitorSelection)
		{
			CComPtr<IVsHierarchy> pHierarchy;
			VSITEMID itemid = VSITEMID_NIL;
			CComPtr<IVsMultiItemSelect> pMultiSelect;
			CComPtr<ISelectionContainer> pSelectionContainer;

			if (SUCCEEDED(pMonitorSelection->GetCurrentSelection(&pHierarchy, &itemid, &pMultiSelect, &pSelectionContainer)))
			{
				if (itemid != VSITEMID_SELECTION && pHierarchy)
				{
					// 单选情况
					CComQIPtr<IVsProject> pProject(pHierarchy);
					if (pProject)
					{
						CComBSTR bstrFullPath;
						if (SUCCEEDED(pProject->GetMkDocument(itemid, &bstrFullPath)))
						{
							CString strPath(bstrFullPath);

							extern void AddFileToChat(const wchar_t* fullPath);
							AddFileToChat(strPath);
						}
					}
				}
				else if (pMultiSelect && pHierarchy)
				{
					// 多选情况
					ULONG cItems = 0;
					BOOL fSingleHierarchy = FALSE;
					if (SUCCEEDED(pMultiSelect->GetSelectionInfo(&cItems, &fSingleHierarchy)) && cItems > 0)
					{
						// 分配内存存储选中项
						VSITEMSELECTION* rgItems = new VSITEMSELECTION[cItems];
						if (rgItems)
						{
							if (SUCCEEDED(pMultiSelect->GetSelectedItems(0, cItems, rgItems)))
							{
								for (ULONG i = 0; i < cItems; i++)
								{
									if (rgItems[i].pHier)
									{
										CComQIPtr<IVsProject> pProject(rgItems[i].pHier);
										if (pProject)
										{
											CComBSTR bstrFullPath;
											if (SUCCEEDED(pProject->GetMkDocument(rgItems[i].itemid, &bstrFullPath)))
											{
												CString strPath(bstrFullPath);

												extern void AddFileToChat(const wchar_t* fullPath);
												AddFileToChat(strPath);
											}
										}
										// 释放引用
										rgItems[i].pHier->Release();
									}
								}
							}
							delete[] rgItems;
						}
					}
				}
			}
		}
	}

private:
	void _CheckAndOpenSolution();
	void _CloseSolution();

	void _InitState(IServiceProvider* pServiceProvider);
	void _ClearState();

	LazyBugPlugInToolWindow m_MyToolWindow;
	LazyBugChangelistsWindow m_changeslistsWindow;
};

// This exposes CLazyBugPlugInPackage for instantiation via DllGetClassObject; however, an instance
// can not be created by CoCreateInstance, as CLazyBugPlugInPackage is specifically registered with
// VS, not the the system in general.
OBJECT_ENTRY_AUTO(CLSID_LazyBugPlugIn, CLazyBugPlugInPackage)
