#include "stdh.h" // Common header, adjust if your project uses a different one
#include <vsshell.h> // For IVsShell, IVsUIShell5 etc.
#include <textmgr.h> // For IVsTextLines, IVsTextLineMarker, marker types, TBS_READONLY

#include "Utils.h"

#include "codediff/CodeDiff.h"
#include "stringparser/stringparser.h"
#include "timer/timer.h"
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
  


// -----------------------------------------------------------------------
// 辅助：通过 IDispatch Automation 模型安全地读取一个属性（字符串）
// -----------------------------------------------------------------------
static std::wstring DispGetStringProp(IDispatch* pDisp, const wchar_t* propName)
{
	if (!pDisp || !propName)
		return {};

	DISPID dispId = DISPID_UNKNOWN;
	OLECHAR* name = const_cast<OLECHAR*>(propName);
	if (FAILED(pDisp->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispId)))
		return {};

	DISPPARAMS dp = {};
	CComVariant result;
	if (FAILED(pDisp->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
		DISPATCH_PROPERTYGET, &dp, &result, nullptr, nullptr)))
		return {};

	if (FAILED(result.ChangeType(VT_BSTR)))
		return {};

	return result.bstrVal ? std::wstring(result.bstrVal) : std::wstring();
}

// -----------------------------------------------------------------------
// 辅助：通过 IDispatch 读取一个子对象属性（返回 IDispatch*）
// -----------------------------------------------------------------------
static CComPtr<IDispatch> DispGetDispProp(IDispatch* pDisp, const wchar_t* propName)
{
	if (!pDisp || !propName)
		return nullptr;

	DISPID dispId = DISPID_UNKNOWN;
	OLECHAR* name = const_cast<OLECHAR*>(propName);
	if (FAILED(pDisp->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispId)))
		return nullptr;

	DISPPARAMS dp = {};
	CComVariant result;
	if (FAILED(pDisp->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
		DISPATCH_PROPERTYGET, &dp, &result, nullptr, nullptr)))
		return nullptr;

	if (result.vt != VT_DISPATCH || !result.pdispVal)
		return nullptr;

	CComPtr<IDispatch> pOut(result.pdispVal);
	return pOut;
}

// -----------------------------------------------------------------------
// 辅助：通过 IDispatch 按索引（1-based）从集合中取元素（IDispatch*）
// -----------------------------------------------------------------------
static CComPtr<IDispatch> DispGetItemByIndex(IDispatch* pDisp, long index)
{
	if (!pDisp)
		return nullptr;

	DISPID dispId = DISPID_UNKNOWN;
	OLECHAR* name = const_cast<OLECHAR*>(L"Item");
	if (FAILED(pDisp->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispId)))
		return nullptr;

	CComVariant arg((long)index);
	DISPPARAMS dp;
	dp.cArgs = 1;
	dp.rgvarg = &arg;
	dp.cNamedArgs = 0;
	dp.rgdispidNamedArgs = nullptr;

	CComVariant result;
	if (FAILED(pDisp->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
		DISPATCH_METHOD | DISPATCH_PROPERTYGET, &dp, &result, nullptr, nullptr)))
		return nullptr;

	if (result.vt != VT_DISPATCH || !result.pdispVal)
		return nullptr;

	CComPtr<IDispatch> pOut(result.pdispVal);
	return pOut;
}

// -----------------------------------------------------------------------
// 辅助：通过 IDispatch 读取集合的 Count 属性
// -----------------------------------------------------------------------
static long DispGetCount(IDispatch* pDisp)
{
	if (!pDisp)
		return 0;

	DISPID dispId = DISPID_UNKNOWN;
	OLECHAR* name = const_cast<OLECHAR*>(L"Count");
	if (FAILED(pDisp->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispId)))
		return 0;

	DISPPARAMS dp = {};
	CComVariant result;
	if (FAILED(pDisp->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
		DISPATCH_PROPERTYGET, &dp, &result, nullptr, nullptr)))
		return 0;

	if (FAILED(result.ChangeType(VT_I4)))
		return 0;

	return result.lVal;
}

// -----------------------------------------------------------------------
// 辅助：将宏展开后的路径字符串（分号分隔）解析为绝对路径列表
// projDir：项目文件所在目录（用于解析相对路径）
// -----------------------------------------------------------------------
static std::vector<std::string> ParseIncludePaths(const std::wstring& rawPaths,
	const std::wstring& projDir)
{
	std::vector<std::string> result;

	// 按分号分割
	std::wstring token;
	std::wistringstream ss(rawPaths);
	while (std::getline(ss, token, L';'))
	{
		// 去掉首尾空白
		auto b = token.find_first_not_of(L" \t\r\n");
		auto e = token.find_last_not_of(L" \t\r\n");
		if (b == std::wstring::npos)
			continue;
		token = token.substr(b, e - b + 1);

		// 跳过残留宏（未展开的 $(xxx)）
		if (token.find(L"$(") != std::wstring::npos)
			continue;

		// 转为绝对路径
		std::wstring absPath;
		if (PathIsRelativeW(token.c_str()))
		{
			wchar_t buf[MAX_PATH] = {};
			PathCombineW(buf, projDir.c_str(), token.c_str());
			absPath = buf;
		}
		else
		{
			absPath = token;
		}

		// 规范化（去掉多余的 ..\ 等）
		wchar_t canonical[MAX_PATH] = {};
		if (PathCanonicalizeW(canonical, absPath.c_str()))
			absPath = canonical;

		// 转小写存储
		std::transform(absPath.begin(), absPath.end(), absPath.begin(), ::towlower);
		result.push_back(widechar_to_utf8(absPath.c_str()));
	}

	return result;
}

// -----------------------------------------------------------------------
// 核心：给定一个项目的 IVsHierarchy，如果是 vcxproj，
// 通过 DTE2 Automation 读取第一个（Active）配置的
// AdditionalIncludeDirectories 和 PrecompiledHeader 相关设置，
// 填充 ProjSetting。
//
// 访问路径（Automation 对象模型）：
//   DTE2
//   └─ Solution
//      └─ Projects (collection)
//         └─ Project  (.FullName == 项目文件绝对路径)
//            └─ Object  → VCProject (IDispatch)
//               └─ Configurations (collection)
//                  └─ Configuration[0]  → VCConfiguration (IDispatch)
//                     └─ Tools (collection)
//                        └─ VCCLCompilerTool (IDispatch)
//                           ├─ AdditionalIncludeDirectories  (BSTR)
//                           ├─ PrecompiledHeaderThrough       (BSTR) ← pch 头文件名
//                           └─ PrecompiledHeaderFile          (BSTR) ← .pch 输出路径
// -----------------------------------------------------------------------




// -----------------------------------------------------------------------
// 辅助：从 DTE ProjectItem 递归收集文件
// -----------------------------------------------------------------------
static void CollectFilesFromProjectItem(IDispatch* pItem, std::set<std::string>& outFiles)
{
	if (!pItem)
		return;

	// 检查该 ProjectItem 是否对应一个物理文件
	// ProjectItem.FileCount 返回 short 类型
	DISPID dispId = DISPID_UNKNOWN;
	OLECHAR* nameFileCount = const_cast<OLECHAR*>(L"FileCount");
	CComVariant varFileCount;
	if (SUCCEEDED(pItem->GetIDsOfNames(IID_NULL, &nameFileCount, 1, LOCALE_USER_DEFAULT, &dispId)))
	{
		DISPPARAMS dp = {};
		if (SUCCEEDED(pItem->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
			DISPATCH_PROPERTYGET, &dp, &varFileCount, nullptr, nullptr)))
		{
			if (FAILED(varFileCount.ChangeType(VT_I4)))
				varFileCount.lVal = 0;
		}
	}

	// 如果 FileCount > 0，读取 FileNames(1)
	if (varFileCount.lVal > 0)
	{
		OLECHAR* nameFileNames = const_cast<OLECHAR*>(L"FileNames");
		if (SUCCEEDED(pItem->GetIDsOfNames(IID_NULL, &nameFileNames, 1, LOCALE_USER_DEFAULT, &dispId)))
		{
			CComVariant argIndex((short)1);  // DTE ProjectItem.FileNames 是 1-based
			DISPPARAMS dpFileNames;
			dpFileNames.cArgs = 1;
			dpFileNames.rgvarg = &argIndex;
			dpFileNames.cNamedArgs = 0;
			dpFileNames.rgdispidNamedArgs = nullptr;

			CComVariant varFileName;
			if (SUCCEEDED(pItem->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
				DISPATCH_PROPERTYGET, &dpFileNames, &varFileName, nullptr, nullptr)))
			{
				if (SUCCEEDED(varFileName.ChangeType(VT_BSTR)) && varFileName.bstrVal)
				{
					std::wstring wpath(varFileName.bstrVal);

					// 判断是否是真实文件
					DWORD attr = GetFileAttributesW(wpath.c_str());
					bool isRealFile = (attr != INVALID_FILE_ATTRIBUTES &&
						!(attr & FILE_ATTRIBUTE_DIRECTORY));
					if (isRealFile)
					{
						std::wstring wpathLower = wpath;
						std::transform(wpathLower.begin(), wpathLower.end(), wpathLower.begin(), ::towlower);
						outFiles.insert(widechar_to_utf8(wpathLower.c_str()));
					}
				}
			}
		}
	}

	// 递归处理子项 (ProjectItem.ProjectItems)
	CComPtr<IDispatch> pSubItems = DispGetDispProp(pItem, L"ProjectItems");
	if (pSubItems)
	{
		long subCount = DispGetCount(pSubItems);
		for (long i = 1; i <= subCount; i++)
		{
			CComPtr<IDispatch> pSubItem = DispGetItemByIndex(pSubItems, i);
			if (pSubItem)
				CollectFilesFromProjectItem(pSubItem, outFiles);
		}
	}
}

// -----------------------------------------------------------------------
// 辅助：从 DTE Project 收集所有文件
// -----------------------------------------------------------------------
static void CollectFilesFromProject(IDispatch* pProj, std::set<std::string>& outFiles)
{
	if (!pProj)
		return;

	// 获取 Project.ProjectItems 集合
	CComPtr<IDispatch> pItems = DispGetDispProp(pProj, L"ProjectItems");
	if (!pItems)
		return;

	long itemCount = DispGetCount(pItems);
	for (long i = 1; i <= itemCount; i++)
	{
		CComPtr<IDispatch> pItem = DispGetItemByIndex(pItems, i);
		if (pItem)
			CollectFilesFromProjectItem(pItem, outFiles);
	}
}

// -----------------------------------------------------------------------
// 辅助：递归枚举所有项目（包括 Solution Folder 中的项目）
// Solution Folder 的 GUID 是 {66A26720-8FB5-11D2-AA7E-00C04F688DDE}
// -----------------------------------------------------------------------
static void CollectProjectsRecursive(IDispatch* pProj, std::vector<CComPtr<IDispatch>>& allProjects)
{
	if (!pProj)
		return;

	// 获取项目的 Kind（类型 GUID）
	std::wstring kindStr = DispGetStringProp(pProj, L"Kind");
	
	// 检查是否是 Solution Folder，GUID 是 {66A26720-8FB5-11D2-AA7E-00C04F688DDE}
	if (_wcsicmp(kindStr.c_str(), L"{66A26720-8FB5-11D2-AA7E-00C04F688DDE}") == 0)
	{
		// 是 Solution Folder，遍历其下的 ProjectItems
		CComPtr<IDispatch> pSubItems = DispGetDispProp(pProj, L"ProjectItems");
		if (pSubItems)
		{
			long subCount = DispGetCount(pSubItems);
			for (long j = 1; j <= subCount; j++)
			{
				CComPtr<IDispatch> pSubItem = DispGetItemByIndex(pSubItems, j);
				if (pSubItem)
				{
					// ProjectItem.SubProject 获取实际的项目对象
					CComPtr<IDispatch> pSubProj = DispGetDispProp(pSubItem, L"SubProject");
					if (pSubProj)
					{
						// 递归处理子项目
						CollectProjectsRecursive(pSubProj, allProjects);
					}
				}
			}
		}
	}
	else
	{
		// 其他常规项目，直接添加
		allProjects.push_back(pProj);
	}
}

static void EnumerateAllProjects(IDispatch* pProjects, std::vector<CComPtr<IDispatch>>& allProjects)
{
	if (!pProjects)
		return;

	long projCount = DispGetCount(pProjects);
	for (long i = 1; i <= projCount; i++)
	{
		CComPtr<IDispatch> pProj = DispGetItemByIndex(pProjects, i);
		if (pProj)
		{
			// 对顶层项目进行递归处理
			CollectProjectsRecursive(pProj, allProjects);
		}
	}
}

// -----------------------------------------------------------------------
// FillVcxprojSetting2 - 直接使用已枚举的 pProj 来访问项目设置
// 参数：
//   dbFolder       - 数据库文件夹路径
//   pProj          - DTE Project 对象 (IDispatch)
//   projFileAbsPath- 项目文件绝对路径
//   setting        - 输出参数：项目设置
//   projFiles      - 项目文件集合（lowerCased 路径）
// -----------------------------------------------------------------------
static void FillVcxprojSetting2(const char* dbFolder, IDispatch* pProj,
	const std::wstring& projFileAbsPath,
	ProjSetting& setting,
	const std::set<std::string>& projFiles)
{
	if (!pProj)
		return;

	// 1. Project.Object → VCProject (IDispatch)
	CComPtr<IDispatch> pVCProject = DispGetDispProp(pProj, L"Object");
	if (!pVCProject)
		return;

	// 2. VCProject.Configurations
	CComPtr<IDispatch> pConfigs = DispGetDispProp(pVCProject, L"Configurations");
	if (!pConfigs)
		return;

	long cfgCount = DispGetCount(pConfigs);
	if (cfgCount <= 0)
		return;

	// 取第一个配置（通常对应当前激活的配置）
	CComPtr<IDispatch> pConfig = DispGetItemByIndex(pConfigs, 1);
	if (!pConfig)
		return;

	// 3. VCConfiguration.Tools → 找 VCCLCompilerTool
	//    Tools 集合里按名字找：Item("VCCLCompilerTool")
	CComPtr<IDispatch> pTools = DispGetDispProp(pConfig, L"Tools");
	if (!pTools)
		return;

	// 按名字取 tool
	CComPtr<IDispatch> pCompilerTool;
	{
		DISPID dispId = DISPID_UNKNOWN;
		OLECHAR* nameItem = const_cast<OLECHAR*>(L"Item");
		if (SUCCEEDED(pTools->GetIDsOfNames(IID_NULL, &nameItem, 1,
			LOCALE_USER_DEFAULT, &dispId)))
		{
			CComVariant argName(L"VCCLCompilerTool");
			DISPPARAMS dp;
			dp.cArgs = 1;
			dp.rgvarg = &argName;
			dp.cNamedArgs = 0;
			dp.rgdispidNamedArgs = nullptr;

			CComVariant result;
			if (SUCCEEDED(pTools->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT,
				DISPATCH_METHOD | DISPATCH_PROPERTYGET,
				&dp, &result, nullptr, nullptr)))
			{
				if (result.vt == VT_DISPATCH && result.pdispVal)
					pCompilerTool = result.pdispVal;
			}
		}
	}

	if (!pCompilerTool)
		return;

	// 4. 读取 AdditionalIncludeDirectories（已展开宏后的字符串）
	//    VCCLCompilerTool 有两个属性：
	//      AdditionalIncludeDirectories  → 原始字符串（含宏）
	//      FullIncludePath               → 展开宏后的完整路径（推荐）
	std::wstring includeRaw = DispGetStringProp(pCompilerTool, L"FullIncludePath");
	if (includeRaw.empty())
		includeRaw = DispGetStringProp(pCompilerTool, L"AdditionalIncludeDirectories");

	// 5. 读取预编译头相关
	//    PrecompiledHeaderThrough → pch 源头文件名（如 "pch.h" / "stdafx.h"）
	//    PrecompiledHeaderFile    → .pch 输出路径（如 "$(OutDir)xxx.pch"）
	std::wstring pchHeader = DispGetStringProp(pCompilerTool, L"PrecompiledHeaderThrough");
	std::wstring pchOutput = DispGetStringProp(pCompilerTool, L"PrecompiledHeaderFile");

	// 6. 把项目目录算出来，用于相对路径解析
	std::wstring projDir = projFileAbsPath;
	auto slashPos = projDir.find_last_of(L"\\/");
	if (slashPos != std::wstring::npos)
		projDir = projDir.substr(0, slashPos);

	// 7. 填充 setting
	if (!includeRaw.empty())
		setting.additionalIncludeFullPathes = ParseIncludePaths(includeRaw, projDir);

	// pch 头文件：只有文件名时，结合项目目录构建绝对路径
	if (!pchHeader.empty() && pchHeader.find(L"$(") == std::wstring::npos)
	{
		std::wstring absHeader;
		if (PathIsRelativeW(pchHeader.c_str()))
		{
			wchar_t buf[MAX_PATH] = {};
			PathCombineW(buf, projDir.c_str(), pchHeader.c_str());
			absHeader = buf;
		}
		else
		{
			absHeader = pchHeader;
		}
		std::transform(absHeader.begin(), absHeader.end(), absHeader.begin(), ::towlower);
		setting.lowerCasedPchFullPath = widechar_to_utf8(absHeader.c_str());

		// 检查 pch 文件是否存在，如果不存在，在项目文件中查找同名文件
		DWORD attr = GetFileAttributesW(absHeader.c_str());
		if (attr == INVALID_FILE_ATTRIBUTES)
		{
			// 转小写用于比较
			std::string pchFileNameLowerUtf8 = GetFileName(setting.lowerCasedPchFullPath);
			StringLower(pchFileNameLowerUtf8);

			setting.lowerCasedPchFullPath = "";

			// 在项目文件集合中查找同名文件（files 中已是 lowerCased 路径）
			for (const auto& lowerFilePath : projFiles)
			{
				std::string fileName = GetFileName(lowerFilePath);

				if (fileName == pchFileNameLowerUtf8)
				{
					// 找到同名文件，更新 pch 路径
					setting.lowerCasedPchFullPath = lowerFilePath;
					break;
				}
			}
		}

		if (!setting.lowerCasedPchFullPath.empty())
		{
			setting.lowerCasedPchOutputFullPath = dbFolder;
			setting.lowerCasedPchOutputFullPath += "\\_pch\\";
			std::string s = setting.lowerCasedPchFullPath;
			RemoveFileSuffix(s);
			std::string name;
			ConvertFullPathToName(s.c_str(), name);
			setting.lowerCasedPchOutputFullPath += name;
			setting.lowerCasedPchOutputFullPath += ".pch";
			StringLower(setting.lowerCasedPchOutputFullPath);
		}
	}
}


// -----------------------------------------------------------------------
// Util_GenerateSolutionDump2: 使用 DTE 接口获取项目内容
// 
// 参数：
//   dbFolder   : 数据库文件夹路径
//   pSolution  : IVsSolution 接口
//   dmp        : 输出的 SolutionDump 结构
//
// 返回：
//   成功返回 true，失败返回 false
// -----------------------------------------------------------------------
bool Util_GenerateSolutionDump2(const char* dbFolder, CComPtr<IVsSolution> pSolution, SolutionDump& dmp, int sliceTime, GenerateSlnDumpProgress& progress)
{
	if (!pSolution)
		return false;

	// ── 0. 获取 DTE2 IDispatch，用于后续读取 Project 属性 ──────────────
	CComPtr<IDispatch> pDTEDisp;
	{
		CComPtr<IServiceProvider> sp = g_ps.pServiceProvider;
		if (sp)
		{
			CComPtr<IUnknown> pDTEUnk;
			// SID_SDTE == IID_DTE (VS 约定)
			static const GUID SID_SDTE =
			{ 0x04A72314, 0x32E9, 0x48E2,
			  { 0x9B, 0x87, 0xA6, 0x36, 0x03, 0x45, 0x4F, 0x3E } };
			if (SUCCEEDED(sp->QueryService(SID_SDTE, IID_IUnknown, (void**)&pDTEUnk)) && pDTEUnk)
				pDTEUnk->QueryInterface(IID_IDispatch, (void**)&pDTEDisp);
		}
	}

	if (!pDTEDisp)
		return false;

	// ── 1. 获取 DTE.Solution ──────────────────────────────────────────
	CComPtr<IDispatch> pDTESolution = DispGetDispProp(pDTEDisp, L"Solution");
	if (!pDTESolution)
		return false;

	// ── 2. 每次都重新枚举所有项目 ────────────────────────────────────
	CComPtr<IDispatch> pProjects = DispGetDispProp(pDTESolution, L"Projects");
	if (!pProjects)
		return false;

	std::vector<CComPtr<IDispatch>> allProjects;
	EnumerateAllProjects(pProjects, allProjects);

// 	if (allProjects.empty())
// 		return false;

	const int totalProjs = (int)allProjects.size();

	// ── 3. 首次调用：初始化 dmp 和 progress ───────────────────────────
	if (progress.IsEmpty())
	{
		dmp.projs.clear();

		// 获取 Solution 的完整路径
		std::wstring solutionPath = DispGetStringProp(pDTESolution, L"FullName");
		if (!solutionPath.empty())
		{
			std::wstring solPathLower = solutionPath;
			std::transform(solPathLower.begin(), solPathLower.end(), solPathLower.begin(), ::towlower);
			dmp.lowerCasedPath = widechar_to_utf8(solPathLower.c_str());
		}

		progress.cur   = 0;
		progress.total = totalProjs;
	}
	else
	{
		// 项目总数发生变化（如新增/卸载了项目），更新 total
		// cur 保持不变，继续从上次的位置处理
		if (progress.total != totalProjs)
			progress.total = totalProjs;

		// 如果已全部完成，直接返回
		if (progress.IsDone())
			return !dmp.projs.empty();
	}

	// ── 4. 记录起始时刻，在 sliceTime 毫秒内处理尽可能多的项目 ──────────
	const AbsTick tickStart = GetAbsTick();

	while (progress.cur < totalProjs)
	{
		// 检查是否已超出本次 slice 的时间预算（GetAbsTick 单位为 ms）
		if ((GetAbsTick() - tickStart) >= (AbsTick)sliceTime)
			break;

		int i = progress.cur;
		CComPtr<IDispatch> pProj = allProjects[i];

		// 推进进度（无论本项目是否有效，都算处理完毕）
		progress.cur++;

		if (!pProj)
			continue;

		SolutionDump::ProjDump projDump;

		// ── 5. 获取项目文件的完整路径 ─────────────────────────────────
		std::wstring projFullPath = DispGetStringProp(pProj, L"FullName");
		if (projFullPath.empty())
			continue; // 跳过没有物理路径的无效项目

		// 规范化路径
		wchar_t canonical[MAX_PATH] = {};
		if (PathCanonicalizeW(canonical, projFullPath.c_str()))
			projFullPath = canonical;

		// 小写路径作为 key
		std::wstring wLower = projFullPath;
		std::transform(wLower.begin(), wLower.end(), wLower.begin(), ::towlower);
		std::string lowerCasedProjPath = widechar_to_utf8(wLower.c_str());

		// ── 6. 判断项目类型 ───────────────────────────────────────────
		bool isVcxproj = false;
		bool isNoExt   = false;

		wchar_t ext[_MAX_EXT] = {};
		_wsplitpath_s(projFullPath.c_str(),
			nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT);
		std::wstring wext = ext;
		std::transform(wext.begin(), wext.end(), wext.begin(), ::towlower);
		isVcxproj = (wext == L".vcxproj");
		if (wext.empty())
			isNoExt = true;

		// ── 7. 收集项目内所有文件 ─────────────────────────────────────
		CollectFilesFromProject(pProj, projDump.files);

		// ── 8. 如果是 vcxproj，读取 include 路径和 pch 设置 ──────────
		if (isVcxproj)
		{
			FillVcxprojSetting2(dbFolder, pProj, projFullPath, projDump.setting, projDump.files);
		}

		// ── 9. 验证项目是否有效 ───────────────────────────────────────
		bool isValidProj = true;
		if (isNoExt)
		{
			if (projDump.files.empty())
				isValidProj = false;
		}

		if (isValidProj)
			dmp.projs[lowerCasedProjPath] = std::move(projDump);
	}

	return !dmp.projs.empty();
}



bool Util_GenerateSolutionDumpTimeStamps(SolutionDump& dmp, SolutionDumpTimeStamps& timeStamps)
{
	timeStamps.projs.clear();
	timeStamps.lowerCasedPath = dmp.lowerCasedPath;

	// 获取 solution 文件的最后修改时间
	std::wstring solPath = utf8_to_widechar(dmp.lowerCasedPath.c_str());
	timeStamps.t = Util_GetFileTime(solPath);

	// 遍历每个项目，提取路径和时间戳
	for (const auto& proj : dmp.projs)
	{
		SolutionDumpTimeStamps::Proj tsProj;
		tsProj.lowerCasedPath = proj.first; // key 即为 lowerCasedPath

		std::wstring projPath = utf8_to_widechar(proj.first.c_str());
		tsProj.t = Util_GetFileTime(projPath);

		timeStamps.projs.push_back(std::move(tsProj));
	}

	return true;
}

bool Util_CheckSolutionDumpOutOfDate(SolutionDumpTimeStamps& timeStamps)
{
	// 检查 solution 文件本身是否被修改
	if (!timeStamps.lowerCasedPath.empty())
	{
		std::wstring solPath = utf8_to_widechar(timeStamps.lowerCasedPath.c_str());
		FILETIME current = Util_GetFileTime(solPath);
		if (!Util_EqualFileTime(current, timeStamps.t))
			return true;
	}

	// 检查每个项目文件是否被修改
	for (const auto& proj : timeStamps.projs)
	{
		if (proj.lowerCasedPath.empty())
			continue;

		std::wstring projPath = utf8_to_widechar(proj.lowerCasedPath.c_str());
		FILETIME current = Util_GetFileTime(projPath);

		// 文件不存在时 GetFileTime 返回零值，如果之前记录的不是零值，说明文件被删除了，也视为过期
		if (!Util_EqualFileTime(current, proj.t))
			return true;
	}

	return false;
}

