#include "stdh.h"

#include "Package.h"

#include "../LazyBugSource/SolutionDump.h"

#include "Utils.h"

#include "CommandFilter.h"

#include "LazyBugHookManager.h"

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

		// 监听文本视图创建，给每个编辑器视图安装复制引用过滤器
		CComObject<CTextViewCreationListener>::CreateInstance(&g_ps.pTextViewCreationListener);
		if (g_ps.pTextViewCreationListener)
		{
			g_ps.pTextViewCreationListener->AddRef();
			if (g_ps.pTextManager)
				g_ps.pTextViewCreationListener->Advise(g_ps.pTextManager);
		}
	}
}

void CLazyBugPlugInPackage::_ClearState()
{
	// 先卸载外部 Hook（在释放 VS 服务之前，避免 Hook 关闭时仍引用已释放的服务）
	SetLazyBugHook(nullptr);
	g_lazyBugHookManager.Unload();

	if (g_ps.pTextViewCreationListener != nullptr)
	{
		g_ps.pTextViewCreationListener->Unadvise();
		g_ps.pTextViewCreationListener->Release();
		g_ps.pTextViewCreationListener = nullptr;
	}

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
	// Controls 的 CloseSolution 内部会触发 Hook 的 OnSolutionClosed
	CloseSolution();

	// 清空 Controls 持有的 Hook 指针，并卸载 Hook DLL
	SetLazyBugHook(nullptr);
	g_lazyBugHookManager.Unload();
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

			// 从 .sln 所在目录加载 LazyBugHook.dll，并把 Hook 指针传给 Controls
			g_lazyBugHookManager.Load((LPCWSTR)bstrSolutionDirectory);
			SetLazyBugHook(g_lazyBugHookManager.GetHook());

			// 调用原来的 OpenSolution 方法（Controls 内部会触发 Hook 的 OnSolutionOpened）
			OpenSolution(solutionPath.c_str());
		}
	}

}
