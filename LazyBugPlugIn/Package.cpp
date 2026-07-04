#include "stdh.h"

#include "Package.h"

#include "../LazyBugSource/SolutionDump.h"

#include "Utils.h"

PackageState g_ps;

void CLazyBugPlugInPackage::_InitState(IServiceProvider* pServiceProvider)
{
//	MessageBox(NULL, L"aa", L"aa", MB_OK);
	if (pServiceProvider)
	{
		g_ps.pServiceProvider = pServiceProvider;
		HRESULT hr = pServiceProvider->QueryService(SID_SVsSolution, IID_IVsSolution, (void**)&g_ps.pSolution);
		if (SUCCEEDED(hr))
		{
			if (g_ps.pSolution)
				hr = g_ps.pSolution->AdviseSolutionEvents(this, &g_ps.dwSolutionEventsCookie);
		}

		pServiceProvider->QueryService(SID_SVsUIShellOpenDocument, IID_IVsUIShellOpenDocument, (void**)&g_ps.pUIShellOpenDocument);
		pServiceProvider->QueryService(SID_SVsTextManager, IID_IVsTextManager, (void**)&g_ps.pTextManager);

		CComObject<CRDTEventsListener>::CreateInstance(&g_ps.pRDTEventsListener);

		if (g_ps.pRDTEventsListener)
			g_ps.pRDTEventsListener->AddRef();
	}
}

void CLazyBugPlugInPackage::_ClearState()
{
	if (g_ps.pRDTEventsListener != nullptr)
	{
		g_ps.pRDTEventsListener->Release();
		g_ps.pRDTEventsListener = nullptr; // Set pointer to null after releasing
	}

	if (g_ps.dwSolutionEventsCookie != VSCOOKIE_NIL && g_ps.pSolution)
		g_ps.pSolution->UnadviseSolutionEvents(g_ps.dwSolutionEventsCookie);
	g_ps.pSolution.Release();
	g_ps.pServiceProvider.Release();
	g_ps.pUIShellOpenDocument.Release();
}

void CLazyBugPlugInPackage::_CloseSolution()
{
	CloseSolution();
}

void CLazyBugPlugInPackage::_CheckAndOpenSolution()
{
	// 获取解决方案信息
	if (g_ps.pSolution)
	{

		CComBSTR bstrSolutionName;
		CComBSTR bstrSolutionDirectory;
		HRESULT hr = g_ps.pSolution->GetSolutionInfo(&bstrSolutionDirectory, &bstrSolutionName, NULL);
		if (SUCCEEDED(hr) && bstrSolutionDirectory)
		{
			// 将BSTR转换为CString
			CString strSolutionPath = bstrSolutionName;

			// 转换为ANSI字符串
			CT2CA pszConvertedAnsiString(strSolutionPath, CP_UTF8);
			std::string solutionPath(pszConvertedAnsiString);

			// 调用原来的 OpenSolution 方法
			OpenSolution(solutionPath.c_str());
		}
	}

}
